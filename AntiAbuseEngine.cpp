#include "AntiAbuseEngine.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <random>
#include <set>

// ==================== SlidingWindow Implementation ====================

void SlidingWindow::addEvent(const std::chrono::system_clock::time_point& timestamp) {
    timestamps.push_back(timestamp);
    cleanup(timestamp);
}

void SlidingWindow::cleanup(const std::chrono::system_clock::time_point& currentTime) {
    auto cutoff = currentTime - windowDuration;
    while (!timestamps.empty() && timestamps.front() < cutoff) {
        timestamps.pop_front();
    }
}

double SlidingWindow::getRate() const {
    if (timestamps.empty()) return 0.0;
    
    // Calculate events per minute
    double seconds = windowDuration.count();
    return (timestamps.size() / seconds) * 60.0;
}

double SlidingWindow::getAverageGapMs() const {
    if (timestamps.size() < 2) return 0.0;
    
    double totalGapMs = 0.0;
    for (size_t i = 1; i < timestamps.size(); i++) {
        auto gap = std::chrono::duration_cast<std::chrono::milliseconds>(
            timestamps[i] - timestamps[i-1]);
        totalGapMs += gap.count();
    }
    
    return totalGapMs / (timestamps.size() - 1);
}

// ==================== CoVotingGraph Implementation ====================

void CoVotingGraph::addVote(const std::string& userId, const std::string& proposalId) {
    // Add user to proposal's voter set
    proposalVoters[proposalId].insert(userId);
    
    // Update adjacency matrix for this proposal's voters
    const auto& voters = proposalVoters[proposalId];
    for (const auto& otherUser : voters) {
        if (otherUser != userId) {
            adjacency[userId][otherUser]++;
            adjacency[otherUser][userId]++;  // Symmetric
        }
    }
}

int CoVotingGraph::getCoVoteCount(const std::string& user1, const std::string& user2) const {
    auto it1 = adjacency.find(user1);
    if (it1 == adjacency.end()) return 0;
    
    auto it2 = it1->second.find(user2);
    if (it2 == it1->second.end()) return 0;
    
    return it2->second;
}

std::vector<std::string> CoVotingGraph::getNeighbors(const std::string& userId) const {
    std::vector<std::string> neighbors;
    auto it = adjacency.find(userId);
    if (it != adjacency.end()) {
        for (const auto& pair : it->second) {
            neighbors.push_back(pair.first);
        }
    }
    return neighbors;
}

int CoVotingGraph::getEdgeCount() const {
    int count = 0;
    for (const auto& pair1 : adjacency) {
        count += pair1.second.size();
    }
    return count / 2;  // Divide by 2 because graph is symmetric
}

std::vector<std::vector<std::string>> CoVotingGraph::detectCommunities(int minCoVotes) {
    std::vector<std::vector<std::string>> communities;
    std::unordered_set<std::string> visited;
    
    // Simple community detection: find connected components with high co-vote counts
    for (const auto& pair1 : adjacency) {
        const std::string& user = pair1.first;
        if (visited.count(user)) continue;
        
        // BFS to find connected component
        std::vector<std::string> community;
        std::queue<std::string> queue;
        queue.push(user);
        visited.insert(user);
        
        while (!queue.empty()) {
            std::string current = queue.front();
            queue.pop();
            community.push_back(current);
            
            auto it = adjacency.find(current);
            if (it != adjacency.end()) {
                for (const auto& neighborPair : it->second) {
                    const std::string& neighbor = neighborPair.first;
                    int coVotes = neighborPair.second;
                    
                    if (!visited.count(neighbor) && coVotes >= minCoVotes) {
                        visited.insert(neighbor);
                        queue.push(neighbor);
                    }
                }
            }
        }
        
        // Only include communities with 2+ members
        if (community.size() >= 2) {
            communities.push_back(community);
        }
    }
    
    return communities;
}

void CoVotingGraph::clear() {
    adjacency.clear();
    proposalVoters.clear();
}

// ==================== AntiAbuseEngine Implementation ====================

AntiAbuseEngine::AntiAbuseEngine(double velThreshold, double deltaThreshold, int windowSeconds)
    : velocityThreshold(velThreshold),
      deltaThresholdMs(deltaThreshold),
      minCoVotesForCollusion(5),
      collusionThreshold(0.7),
      botLikelihoodThreshold(0.7),
      velocityWindowSeconds(windowSeconds) {
}

void AntiAbuseEngine::recordVoteEvent(const std::string& userId,
                                     const std::string& proposalId,
                                     const std::chrono::system_clock::time_point& timestamp,
                                     const std::string& ipHash,
                                     const std::string& deviceHash) {
    // Create vote event
    VoteEvent event("VOTE_" + std::to_string(userVoteHistory[userId].size()),
                   userId, proposalId, timestamp, ipHash, deviceHash);
    
    // Add to user history
    userVoteHistory[userId].push_back(event);
    
    // Update velocity window
    if (userVelocityWindows.find(userId) == userVelocityWindows.end()) {
        userVelocityWindows[userId] = SlidingWindow(velocityWindowSeconds);
    }
    userVelocityWindows[userId].addEvent(timestamp);
    
    // Track IP and device
    if (!ipHash.empty()) {
        userIPs[userId].insert(ipHash);
        ipToUsers[ipHash].push_back(userId);
    }
    if (!deviceHash.empty()) {
        userDevices[userId].insert(deviceHash);
        deviceToUsers[deviceHash].push_back(userId);
    }
    
    // Update co-voting graph
    coVotingGraph.addVote(userId, proposalId);
    
    // Update bot detection for this user
    updateBotDetection(userId);
}

double AntiAbuseEngine::calculateVotingVelocity(const std::string& userId) {
    auto it = userVelocityWindows.find(userId);
    if (it != userVelocityWindows.end()) {
        return it->second.getRate();
    }
    return 0.0;
}

double AntiAbuseEngine::calculateAvgInterVoteGap(const std::string& userId) {
    auto it = userVelocityWindows.find(userId);
    if (it != userVelocityWindows.end()) {
        return it->second.getAverageGapMs();
    }
    return 0.0;
}

bool AntiAbuseEngine::detectBotBehavior(const std::string& userId) {
    double velocity = calculateVotingVelocity(userId);
    double avgGap = calculateAvgInterVoteGap(userId);
    
    return (velocity > velocityThreshold) || 
           (avgGap > 0 && avgGap < deltaThresholdMs);
}

void AntiAbuseEngine::updateBotDetection(const std::string& userId) {
    BotDetectionResult result;
    result.userId = userId;
    result.votingVelocity = calculateVotingVelocity(userId);
    result.avgInterVoteGapMs = calculateAvgInterVoteGap(userId);
    result.deviceDiversity = userDevices[userId].size();
    result.ipDiversity = userIPs[userId].size();
    result.isSuspicious = detectBotBehavior(userId);
    
    // Calculate bot likelihood score (0-1)
    double velocityScore = std::min(1.0, result.votingVelocity / (velocityThreshold * 2));
    double gapScore = (result.avgInterVoteGapMs > 0 && result.avgInterVoteGapMs < deltaThresholdMs) 
                      ? (1.0 - result.avgInterVoteGapMs / deltaThresholdMs) : 0.0;
    double deviceScore = (result.deviceDiversity == 1) ? 0.3 : 0.0;  // Single device suspicious
    double ipScore = (result.ipDiversity == 1) ? 0.2 : 0.0;          // Single IP suspicious
    
    result.botLikelihood = std::min(1.0, 
        0.4 * velocityScore + 0.4 * gapScore + deviceScore + ipScore);
    
    // Generate reason
    std::stringstream ss;
    if (result.votingVelocity > velocityThreshold) {
        ss << "High velocity (" << std::fixed << std::setprecision(1) 
           << result.votingVelocity << " votes/min). ";
    }
    if (result.avgInterVoteGapMs > 0 && result.avgInterVoteGapMs < deltaThresholdMs) {
        ss << "Low inter-vote gap (" << std::fixed << std::setprecision(0) 
           << result.avgInterVoteGapMs << "ms). ";
    }
    if (result.deviceDiversity == 1 && userVoteHistory[userId].size() > 10) {
        ss << "Single device used. ";
    }
    result.reason = ss.str();
    
    // Cache result
    botDetectionCache[userId] = result;
    
    // Generate alert if suspicious
    if (result.isSuspicious && result.botLikelihood > botLikelihoodThreshold) {
        generateThreatAlert("bot_detected", result.botLikelihood,
                          {userId}, result.reason);
        markUserSuspicious(userId, result.reason);
    }
}

BotDetectionResult AntiAbuseEngine::detectBot(const std::string& userId) {
    // Check cache first
    auto it = botDetectionCache.find(userId);
    if (it != botDetectionCache.end()) {
        return it->second;
    }
    
    // Calculate and cache
    updateBotDetection(userId);
    return botDetectionCache[userId];
}

std::vector<BotDetectionResult> AntiAbuseEngine::detectAllBots() {
    std::vector<BotDetectionResult> results;
    
    for (const auto& pair : userVoteHistory) {
        const std::string& userId = pair.first;
        BotDetectionResult result = detectBot(userId);
        if (result.isSuspicious) {
            results.push_back(result);
        }
    }
    
    // Sort by bot likelihood (descending)
    std::sort(results.begin(), results.end(),
              [](const BotDetectionResult& a, const BotDetectionResult& b) {
                  return a.botLikelihood > b.botLikelihood;
              });
    
    return results;
}

void AntiAbuseEngine::updateCollusionDetection() {
    collusionDetectionCache.clear();
    
    // Detect communities (suspicious groups)
    auto communities = coVotingGraph.detectCommunities(minCoVotesForCollusion);
    
    for (const auto& community : communities) {
        if (community.size() < 2) continue;
        
        CollusionDetectionResult result;
        result.userGroup = community;
        
        // Calculate collusion metrics
        int totalCoVotes = 0;
        int edgeCount = 0;
        for (size_t i = 0; i < community.size(); i++) {
            for (size_t j = i + 1; j < community.size(); j++) {
                int coVotes = coVotingGraph.getCoVoteCount(community[i], community[j]);
                if (coVotes > 0) {
                    totalCoVotes += coVotes;
                    edgeCount++;
                }
            }
        }
        
        result.coVoteCount = totalCoVotes;
        
        // Calculate collusion score based on density and co-vote count
        int maxPossibleEdges = (community.size() * (community.size() - 1)) / 2;
        double density = (maxPossibleEdges > 0) ? 
                        static_cast<double>(edgeCount) / maxPossibleEdges : 0.0;
        double avgCoVotes = (edgeCount > 0) ? 
                           static_cast<double>(totalCoVotes) / edgeCount : 0.0;
        
        result.coVoteRate = density;
        result.collusionScore = std::min(1.0, 0.5 * density + 0.5 * (avgCoVotes / 20.0));
        result.isSuspicious = result.collusionScore > collusionThreshold;
        
        // Generate description
        std::stringstream ss;
        ss << "Group of " << community.size() << " users with " 
           << totalCoVotes << " co-votes (density: " 
           << std::fixed << std::setprecision(2) << density << ")";
        result.description = ss.str();
        
        collusionDetectionCache.push_back(result);
        
        // Generate alert if suspicious
        if (result.isSuspicious) {
            generateThreatAlert("collusion_detected", result.collusionScore,
                              community, result.description);
            for (const auto& userId : community) {
                markUserSuspicious(userId, "Part of collusion group");
            }
        }
    }
}

std::vector<CollusionDetectionResult> AntiAbuseEngine::detectCollusion() {
    updateCollusionDetection();
    return collusionDetectionCache;
}

double AntiAbuseEngine::calculateAccountAgeScore(const std::string& userId) {
    // Simplified: assume older accounts are more trustworthy
    // In production, use actual account creation date
    int voteCount = userVoteHistory[userId].size();
    return std::min(1.0, voteCount / 50.0);  // Max at 50 votes
}

double AntiAbuseEngine::calculateMajorityAgreementScore(const std::string& userId) {
    // Simplified: in production, compare user's votes with majority votes
    // For now, return neutral score
    return 0.5;
}

UserCredibilityScore AntiAbuseEngine::calculateUserCredibility(const std::string& userId) {
    UserCredibilityScore score;
    score.userId = userId;
    
    // Get bot detection result
    BotDetectionResult botResult = detectBot(userId);
    score.botLikelihood = botResult.botLikelihood;
    
    // Account age score
    score.accountAgeScore = calculateAccountAgeScore(userId);
    
    // Device diversity score (inverse - less diversity is better for trust)
    int deviceCount = userDevices[userId].size();
    score.deviceDiversityScore = (deviceCount == 0) ? 0.5 :
                                 (deviceCount == 1) ? 0.8 :
                                 (deviceCount == 2) ? 0.6 : 0.3;
    
    // Majority agreement score
    score.majorityAgreementScore = calculateMajorityAgreementScore(userId);
    
    // Verification score (simplified - in production, check email/phone verification)
    score.verificationScore = 0.5;
    
    // Collusion score (check if user is in any collusion group)
    score.collusionScore = 0.0;
    for (const auto& collusionResult : collusionDetectionCache) {
        for (const auto& user : collusionResult.userGroup) {
            if (user == userId) {
                score.collusionScore = std::max(score.collusionScore, 
                                               collusionResult.collusionScore);
            }
        }
    }
    
    // Consistency score (default neutral)
    score.consistencyScore = 0.5;
    
    // Report score (default good - 1.0 means no reports)
    score.reportScore = 1.0;
    
    // Calculate overall trust score
    // Weighted combination of factors
    score.trustScore = 
        0.20 * score.accountAgeScore +
        0.15 * score.deviceDiversityScore +
        0.15 * score.majorityAgreementScore +
        0.10 * score.verificationScore +
        0.15 * (1.0 - score.botLikelihood) +           // Inverse
        0.15 * (1.0 - score.collusionScore) +          // Inverse
        0.05 * score.consistencyScore +
        0.05 * score.reportScore;
    
    // Cache result
    userCredibilityScores[userId] = score;
    
    return score;
}

void AntiAbuseEngine::calculateAllCredibilityScores() {
    // Update collusion detection first
    updateCollusionDetection();
    
    // Calculate for all users
    for (const auto& pair : userVoteHistory) {
        calculateUserCredibility(pair.first);
    }
}

double AntiAbuseEngine::getUserTrustScore(const std::string& userId) {
    auto it = userCredibilityScores.find(userId);
    if (it != userCredibilityScores.end()) {
        return it->second.trustScore;
    }
    
    // Calculate and cache
    UserCredibilityScore score = calculateUserCredibility(userId);
    return score.trustScore;
}

void AntiAbuseEngine::markUserSuspicious(const std::string& userId, const std::string& reason) {
    suspiciousUsers[userId] = true;
}

bool AntiAbuseEngine::isUserSuspicious(const std::string& userId) const {
    auto it = suspiciousUsers.find(userId);
    return it != suspiciousUsers.end() && it->second;
}

void AntiAbuseEngine::generateThreatAlert(const std::string& alertType,
                                         double severity,
                                         const std::vector<std::string>& users,
                                         const std::string& description) {
    ThreatAlert alert;
    
    static int alertCounter = 0;
    alert.alertId = "ALERT_" + std::to_string(++alertCounter);
    alert.alertType = alertType;
    alert.severity = severity;
    alert.involvedUsers = users;
    alert.description = description;
    alert.timestamp = std::chrono::system_clock::now();
    alert.resolved = false;
    
    threatAlerts.push_back(alert);
}

std::vector<ThreatAlert> AntiAbuseEngine::getThreatAlerts(bool unresolvedOnly) const {
    if (!unresolvedOnly) {
        return threatAlerts;
    }
    
    std::vector<ThreatAlert> unresolved;
    for (const auto& alert : threatAlerts) {
        if (!alert.resolved) {
            unresolved.push_back(alert);
        }
    }
    return unresolved;
}

void AntiAbuseEngine::resolveThreatAlert(const std::string& alertId) {
    for (auto& alert : threatAlerts) {
        if (alert.alertId == alertId) {
            alert.resolved = true;
            break;
        }
    }
}

std::string AntiAbuseEngine::performSecurityScan() {
    std::stringstream ss;
    
    ss << "\n=== Security Scan Report ===\n\n";
    
    // Bot detection
    auto botResults = detectAllBots();
    ss << "Bot Detection:\n";
    ss << "  Suspicious users: " << botResults.size() << "\n";
    for (size_t i = 0; i < std::min(size_t(5), botResults.size()); i++) {
        const auto& bot = botResults[i];
        ss << "  - " << bot.userId 
           << " (likelihood: " << std::fixed << std::setprecision(2) << bot.botLikelihood
           << ", velocity: " << bot.votingVelocity << " votes/min)\n";
    }
    ss << "\n";
    
    // Collusion detection
    auto collusionResults = detectCollusion();
    ss << "Collusion Detection:\n";
    ss << "  Suspicious groups: " << collusionResults.size() << "\n";
    for (size_t i = 0; i < std::min(size_t(3), collusionResults.size()); i++) {
        const auto& collusion = collusionResults[i];
        ss << "  - Group of " << collusion.userGroup.size() << " users "
           << "(score: " << std::fixed << std::setprecision(2) << collusion.collusionScore
           << ", co-votes: " << collusion.coVoteCount << ")\n";
    }
    ss << "\n";
    
    // Threat alerts
    auto alerts = getThreatAlerts(true);
    ss << "Active Threat Alerts: " << alerts.size() << "\n\n";
    
    return ss.str();
}

std::string AntiAbuseEngine::getSecurityStatistics() const {
    std::stringstream ss;
    
    ss << "\n=== Anti-Abuse Engine Statistics ===\n\n";
    ss << "Total users tracked: " << userVoteHistory.size() << "\n";
    ss << "Suspicious users: " << suspiciousUsers.size() << "\n";
    ss << "Total threat alerts: " << threatAlerts.size() << "\n";
    ss << "Unresolved alerts: " << getThreatAlerts(true).size() << "\n";
    ss << "Co-voting graph edges: " << coVotingGraph.getEdgeCount() << "\n\n";
    
    ss << "Configuration:\n";
    ss << "  Velocity threshold: " << velocityThreshold << " votes/min\n";
    ss << "  Inter-vote gap threshold: " << deltaThresholdMs << " ms\n";
    ss << "  Collusion threshold: " << collusionThreshold << "\n";
    ss << "  Bot likelihood threshold: " << botLikelihoodThreshold << "\n";
    
    return ss.str();
}

int AntiAbuseEngine::getVoteCountInWindow(const std::string& userId, int windowSeconds) const {
    auto it = userVoteHistory.find(userId);
    if (it == userVoteHistory.end()) return 0;
    
    auto now = std::chrono::system_clock::now();
    auto cutoff = now - std::chrono::seconds(windowSeconds);
    
    int count = 0;
    for (const auto& event : it->second) {
        if (event.timestamp >= cutoff) {
            count++;
        }
    }
    
    return count;
}

std::vector<std::string> AntiAbuseEngine::getUsersWithSameIP(const std::string& ipHash) const {
    auto it = ipToUsers.find(ipHash);
    if (it != ipToUsers.end()) {
        return it->second;
    }
    return {};
}

std::vector<std::string> AntiAbuseEngine::getUsersWithSameDevice(const std::string& deviceHash) const {
    auto it = deviceToUsers.find(deviceHash);
    if (it != deviceToUsers.end()) {
        return it->second;
    }
    return {};
}

void AntiAbuseEngine::configureThresholds(double velThreshold,
                                         double deltaThreshold,
                                         double collusionThresh,
                                         double botThresh) {
    velocityThreshold = velThreshold;
    deltaThresholdMs = deltaThreshold;
    collusionThreshold = collusionThresh;
    botLikelihoodThreshold = botThresh;
}

std::string AntiAbuseEngine::getConfiguration() const {
    std::stringstream ss;
    ss << "Anti-Abuse Engine Configuration:\n";
    ss << "  Velocity Threshold: " << velocityThreshold << " votes/min\n";
    ss << "  Inter-vote Gap Threshold: " << deltaThresholdMs << " ms\n";
    ss << "  Min Co-votes for Collusion: " << minCoVotesForCollusion << "\n";
    ss << "  Collusion Threshold: " << collusionThreshold << "\n";
    ss << "  Bot Likelihood Threshold: " << botLikelihoodThreshold << "\n";
    ss << "  Velocity Window: " << velocityWindowSeconds << " seconds\n";
    return ss.str();
}

void AntiAbuseEngine::clearAll() {
    userVoteHistory.clear();
    userVelocityWindows.clear();
    userIPs.clear();
    userDevices.clear();
    ipToUsers.clear();
    deviceToUsers.clear();
    coVotingGraph.clear();
    botDetectionCache.clear();
    collusionDetectionCache.clear();
    userCredibilityScores.clear();
    threatAlerts.clear();
    suspiciousUsers.clear();
}

void AntiAbuseEngine::clearUser(const std::string& userId) {
    userVoteHistory.erase(userId);
    userVelocityWindows.erase(userId);
    userIPs.erase(userId);
    userDevices.erase(userId);
    botDetectionCache.erase(userId);
    userCredibilityScores.erase(userId);
    suspiciousUsers.erase(userId);
}
