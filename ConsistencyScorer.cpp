#include "ConsistencyScorer.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

// Initialize static weight variables (default from documentation)
double WeightedRankingScore::alpha = 0.55;   // relevance
double WeightedRankingScore::beta = 0.25;    // consistency
double WeightedRankingScore::gamma = 0.15;   // trust
double WeightedRankingScore::delta = 0.05;   // recency

// Constructor
ConsistencyScorer::ConsistencyScorer(int windowSize, bool useWindow)
    : rollingWindowSize(windowSize),
      newUserDefaultConsistency(0.5),
      useRollingWindow(useWindow) {
}

// Helper: Calculate mean
double ConsistencyScorer::calculateMean(const std::vector<double>& values) const {
    if (values.empty()) return 0.0;
    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return sum / values.size();
}

// Helper: Calculate standard deviation
double ConsistencyScorer::calculateStdDev(const std::vector<double>& values, double mean) const {
    if (values.size() <= 1) return 0.0;
    
    double variance = calculateVariance(values, mean);
    return std::sqrt(variance);
}

// Helper: Calculate variance
double ConsistencyScorer::calculateVariance(const std::vector<double>& values, double mean) const {
    if (values.size() <= 1) return 0.0;
    
    double sumSquaredDiff = 0.0;
    for (double value : values) {
        double diff = value - mean;
        sumSquaredDiff += diff * diff;
    }
    
    return sumSquaredDiff / (values.size() - 1);
}

// Record a proposal's similarity score for a user
void ConsistencyScorer::recordProposalSimilarity(const std::string& userId,
                                                 const std::string& proposalId,
                                                 double similarityScore,
                                                 const std::string& timestamp,
                                                 const std::string& topicId) {
    ProposalSimilarityRecord record(proposalId, similarityScore, timestamp, topicId);
    
    // Add to user's history
    userProposalHistory[userId].push_back(record);
    
    // Maintain rolling window if enabled
    if (useRollingWindow && userProposalHistory[userId].size() > static_cast<size_t>(rollingWindowSize)) {
        // Remove oldest entry
        userProposalHistory[userId].erase(userProposalHistory[userId].begin());
    }
    
    // Update consistency metrics for this user
    updateUserConsistency(userId);
}

// Calculate consistency metrics for a user
ConsistencyMetrics ConsistencyScorer::calculateConsistencyMetrics(const std::string& userId) {
    ConsistencyMetrics metrics(userId);
    
    // Check if user exists in history
    if (userProposalHistory.find(userId) == userProposalHistory.end() ||
        userProposalHistory[userId].empty()) {
        // New user or no history - return default
        metrics.consistencyScore = newUserDefaultConsistency;
        return metrics;
    }
    
    // Extract similarity scores from history
    std::vector<double> similarities;
    for (const auto& record : userProposalHistory[userId]) {
        similarities.push_back(record.similarityScore);
    }
    
    metrics.proposalCount = similarities.size();
    
    // Calculate mean (μ_i)
    metrics.meanSimilarity = calculateMean(similarities);
    
    // Calculate standard deviation (σ_i)
    metrics.stdDevSimilarity = calculateStdDev(similarities, metrics.meanSimilarity);
    
    // Calculate variance (σ_i²)
    metrics.variance = calculateVariance(similarities, metrics.meanSimilarity);
    
    // Calculate consistency score: 1 / (1 + σ_i)
    // This formula ensures:
    // - High variance (erratic) → low consistency score (0.1-0.3)
    // - Low variance (stable) → high consistency score (0.9-1.0)
    // - Bounded in (0, 1]
    metrics.consistencyScore = 1.0 / (1.0 + metrics.stdDevSimilarity);
    
    return metrics;
}

// Get cached consistency score for a user
double ConsistencyScorer::getUserConsistencyScore(const std::string& userId) {
    // Check cache first
    if (userConsistencyCache.find(userId) != userConsistencyCache.end()) {
        return userConsistencyCache[userId].consistencyScore;
    }
    
    // Calculate and cache if not found
    ConsistencyMetrics metrics = calculateConsistencyMetrics(userId);
    userConsistencyCache[userId] = metrics;
    
    return metrics.consistencyScore;
}

// Get full consistency metrics for a user
ConsistencyMetrics ConsistencyScorer::getUserConsistencyMetrics(const std::string& userId) {
    // Check cache first
    if (userConsistencyCache.find(userId) != userConsistencyCache.end()) {
        return userConsistencyCache[userId];
    }
    
    // Calculate and cache if not found
    ConsistencyMetrics metrics = calculateConsistencyMetrics(userId);
    userConsistencyCache[userId] = metrics;
    
    return metrics;
}

// Update consistency metrics for a specific user
void ConsistencyScorer::updateUserConsistency(const std::string& userId) {
    ConsistencyMetrics metrics = calculateConsistencyMetrics(userId);
    userConsistencyCache[userId] = metrics;
}

// Update consistency metrics for all users
void ConsistencyScorer::updateAllConsistencyScores() {
    for (const auto& pair : userProposalHistory) {
        const std::string& userId = pair.first;
        updateUserConsistency(userId);
    }
}

// Calculate weighted ranking score for a proposal
WeightedRankingScore ConsistencyScorer::calculateWeightedScore(const std::string& proposalId,
                                                              const std::string& userId,
                                                              double relevanceScore,
                                                              double trustScore,
                                                              double recencyScore) {
    WeightedRankingScore score;
    score.proposalId = proposalId;
    score.userId = userId;
    score.relevanceScore = relevanceScore;
    score.trustScore = trustScore;
    score.recencyScore = recencyScore;
    
    // Get consistency score for user
    score.consistencyScore = getUserConsistencyScore(userId);
    
    // Calculate final weighted score using formula:
    // final = α*relevance + β*consistency + γ*trust + δ*recency
    score.calculateFinalScore();
    
    return score;
}

// Get proposal count for a user
int ConsistencyScorer::getUserProposalCount(const std::string& userId) const {
    auto it = userProposalHistory.find(userId);
    if (it != userProposalHistory.end()) {
        return it->second.size();
    }
    return 0;
}

// Clear history for a specific user
void ConsistencyScorer::clearUserHistory(const std::string& userId) {
    userProposalHistory.erase(userId);
    userConsistencyCache.erase(userId);
}

// Clear all history
void ConsistencyScorer::clearAllHistory() {
    userProposalHistory.clear();
    userConsistencyCache.clear();
}

// Get all users with consistency scores
std::vector<std::string> ConsistencyScorer::getAllTrackedUsers() const {
    std::vector<std::string> users;
    for (const auto& pair : userConsistencyCache) {
        users.push_back(pair.first);
    }
    return users;
}

// Get users with high consistency
std::vector<std::string> ConsistencyScorer::getHighConsistencyUsers(double threshold) const {
    std::vector<std::string> users;
    for (const auto& pair : userConsistencyCache) {
        if (pair.second.consistencyScore > threshold) {
            users.push_back(pair.first);
        }
    }
    return users;
}

// Get users with low consistency
std::vector<std::string> ConsistencyScorer::getLowConsistencyUsers(double threshold) const {
    std::vector<std::string> users;
    for (const auto& pair : userConsistencyCache) {
        if (pair.second.consistencyScore < threshold) {
            users.push_back(pair.first);
        }
    }
    return users;
}

// Get statistics about consistency scores
std::string ConsistencyScorer::getConsistencyStatistics() const {
    std::stringstream ss;
    
    ss << "\n=== Consistency Scoring Statistics ===\n\n";
    ss << "Total users tracked: " << userConsistencyCache.size() << "\n\n";
    
    if (userConsistencyCache.empty()) {
        ss << "No users tracked yet.\n";
        return ss.str();
    }
    
    // Calculate overall statistics
    std::vector<double> allConsistencyScores;
    std::vector<double> allMeans;
    std::vector<double> allStdDevs;
    
    for (const auto& pair : userConsistencyCache) {
        const ConsistencyMetrics& metrics = pair.second;
        allConsistencyScores.push_back(metrics.consistencyScore);
        allMeans.push_back(metrics.meanSimilarity);
        allStdDevs.push_back(metrics.stdDevSimilarity);
    }
    
    double avgConsistency = calculateMean(allConsistencyScores);
    double avgMeanSim = calculateMean(allMeans);
    double avgStdDev = calculateMean(allStdDevs);
    
    ss << "Average Consistency Score: " << std::fixed << std::setprecision(3) 
       << avgConsistency << "\n";
    ss << "Average Mean Similarity: " << avgMeanSim << "\n";
    ss << "Average Std Deviation: " << avgStdDev << "\n\n";
    
    // Count users by consistency level
    int highConsistency = getHighConsistencyUsers(0.7).size();
    int mediumConsistency = 0;
    int lowConsistency = getLowConsistencyUsers(0.3).size();
    mediumConsistency = userConsistencyCache.size() - highConsistency - lowConsistency;
    
    ss << "Users by consistency level:\n";
    ss << "  High (>0.7):   " << highConsistency << "\n";
    ss << "  Medium (0.3-0.7): " << mediumConsistency << "\n";
    ss << "  Low (<0.3):    " << lowConsistency << "\n\n";
    
    // Show top 5 most consistent users
    std::vector<std::pair<std::string, double>> userScores;
    for (const auto& pair : userConsistencyCache) {
        userScores.push_back({pair.first, pair.second.consistencyScore});
    }
    
    std::sort(userScores.begin(), userScores.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    ss << "Top 5 most consistent users:\n";
    int count = 0;
    for (const auto& pair : userScores) {
        if (count >= 5) break;
        auto it = userConsistencyCache.find(pair.first);
        if (it != userConsistencyCache.end()) {
            const ConsistencyMetrics& metrics = it->second;
            ss << "  " << pair.first 
               << " - Consistency: " << std::fixed << std::setprecision(3) << metrics.consistencyScore
               << " (μ=" << metrics.meanSimilarity 
               << ", σ=" << metrics.stdDevSimilarity
               << ", n=" << metrics.proposalCount << ")\n";
        }
        count++;
    }
    
    return ss.str();
}

// Configure scoring weights
void ConsistencyScorer::configureWeights(double alphaRelevance,
                                        double betaConsistency,
                                        double gammaTrust,
                                        double deltaRecency) {
    // Validate that weights sum to approximately 1.0
    double sum = alphaRelevance + betaConsistency + gammaTrust + deltaRecency;
    if (std::abs(sum - 1.0) > 0.01) {
        std::cerr << "Warning: Weights do not sum to 1.0 (sum=" << sum << ")\n";
    }
    
    WeightedRankingScore::alpha = alphaRelevance;
    WeightedRankingScore::beta = betaConsistency;
    WeightedRankingScore::gamma = gammaTrust;
    WeightedRankingScore::delta = deltaRecency;
}

// Get current weight configuration
std::string ConsistencyScorer::getWeightConfiguration() {
    std::stringstream ss;
    ss << "Weighted Ranking Score Configuration:\n";
    ss << "  α (Relevance):   " << std::fixed << std::setprecision(2) 
       << WeightedRankingScore::alpha << "\n";
    ss << "  β (Consistency): " << WeightedRankingScore::beta << "\n";
    ss << "  γ (Trust):       " << WeightedRankingScore::gamma << "\n";
    ss << "  δ (Recency):     " << WeightedRankingScore::delta << "\n";
    ss << "  Sum: " << (WeightedRankingScore::alpha + WeightedRankingScore::beta +
                        WeightedRankingScore::gamma + WeightedRankingScore::delta) << "\n";
    return ss.str();
}

// Set rolling window size
void ConsistencyScorer::setRollingWindowSize(int windowSize) {
    rollingWindowSize = windowSize;
}

// Set whether to use rolling window
void ConsistencyScorer::setUseRollingWindow(bool useWindow) {
    useRollingWindow = useWindow;
}
