#include "IntelligenceEngine.h"
#include "VotingSystem.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>
#include <ctime>

// Initialize static word lists for NLP
std::vector<std::string> NLPUtils::positiveWords = {
    "good", "great", "excellent", "amazing", "wonderful", "fantastic", "awesome", "brilliant",
    "outstanding", "superb", "perfect", "love", "like", "enjoy", "happy", "pleased",
    "satisfied", "beneficial", "helpful", "useful", "valuable", "important", "necessary",
    "effective", "efficient", "innovative", "creative", "smart", "intelligent", "wise"
};

std::vector<std::string> NLPUtils::negativeWords = {
    "bad", "terrible", "awful", "horrible", "disgusting", "hate", "dislike", "angry",
    "frustrated", "disappointed", "useless", "worthless", "harmful", "dangerous", "risky",
    "expensive", "costly", "difficult", "hard", "impossible", "unfair", "unjust",
    "wrong", "incorrect", "false", "misleading", "confusing", "complicated", "boring"
};

std::vector<std::string> NLPUtils::stopWords = {
    "the", "a", "an", "and", "or", "but", "in", "on", "at", "to", "for", "of", "with",
    "by", "is", "are", "was", "were", "be", "been", "have", "has", "had", "do", "does",
    "did", "will", "would", "could", "should", "may", "might", "can", "this", "that",
    "these", "those", "i", "you", "he", "she", "it", "we", "they", "me", "him", "her"
};

// NLPUtils implementation
void NLPUtils::initializeWordLists() {
    // Word lists are already initialized as static members
}

std::vector<std::string> NLPUtils::tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string lowerText = toLowerCase(text);
    
    std::regex wordRegex(R"(\b\w+\b)");
    std::sregex_iterator iter(lowerText.begin(), lowerText.end(), wordRegex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        tokens.push_back(iter->str());
    }
    
    return tokens;
}

std::vector<std::string> NLPUtils::removeStopWords(const std::vector<std::string>& tokens) {
    std::vector<std::string> filtered;
    std::set<std::string> stopWordSet(stopWords.begin(), stopWords.end());
    
    for (const auto& token : tokens) {
        if (stopWordSet.find(token) == stopWordSet.end()) {
            filtered.push_back(token);
        }
    }
    
    return filtered;
}

std::string NLPUtils::toLowerCase(const std::string& text) {
    std::string result = text;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

double NLPUtils::calculateSimilarity(const std::string& text1, const std::string& text2) {
    auto tokens1 = removeStopWords(tokenize(text1));
    auto tokens2 = removeStopWords(tokenize(text2));
    
    std::set<std::string> set1(tokens1.begin(), tokens1.end());
    std::set<std::string> set2(tokens2.begin(), tokens2.end());
    
    std::set<std::string> intersection;
    std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(),
                         std::inserter(intersection, intersection.begin()));
    
    std::set<std::string> unionSet;
    std::set_union(set1.begin(), set1.end(), set2.begin(), set2.end(),
                   std::inserter(unionSet, unionSet.begin()));
    
    if (unionSet.empty()) return 0.0;
    
    return static_cast<double>(intersection.size()) / unionSet.size();
}

std::vector<std::string> NLPUtils::extractKeywords(const std::string& text, int maxKeywords) {
    auto tokens = removeStopWords(tokenize(text));
    std::unordered_map<std::string, int> frequency;
    
    for (const auto& token : tokens) {
        if (token.length() > 2) {  // Only consider words longer than 2 characters
            frequency[token]++;
        }
    }
    
    std::vector<std::pair<std::string, int>> sortedWords(frequency.begin(), frequency.end());
    std::sort(sortedWords.begin(), sortedWords.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::vector<std::string> keywords;
    for (int i = 0; i < std::min(maxKeywords, static_cast<int>(sortedWords.size())); ++i) {
        keywords.push_back(sortedWords[i].first);
    }
    
    return keywords;
}

// SentimentAnalyzer implementation
SentimentAnalyzer::SentimentAnalyzer() {
    // Initialize word sentiments
    for (const auto& word : NLPUtils::positiveWords) {
        wordSentiments[word] = 0.8;
    }
    for (const auto& word : NLPUtils::negativeWords) {
        wordSentiments[word] = -0.8;
    }
}

SentimentScore SentimentAnalyzer::analyzeSentiment(const std::string& text) {
    SentimentScore score;
    auto tokens = NLPUtils::removeStopWords(NLPUtils::tokenize(text));
    
    if (tokens.empty()) {
        return score;  // Return neutral score
    }
    
    double totalSentiment = 0.0;
    int sentimentWords = 0;
    int positiveCount = 0;
    int negativeCount = 0;
    
    for (const auto& token : tokens) {
        auto it = wordSentiments.find(token);
        if (it != wordSentiments.end()) {
            totalSentiment += it->second;
            sentimentWords++;
            
            if (it->second > 0) positiveCount++;
            else if (it->second < 0) negativeCount++;
        }
    }
    
    if (sentimentWords > 0) {
        score.overall = totalSentiment / sentimentWords;
        score.positivity = static_cast<double>(positiveCount) / tokens.size();
        score.negativity = static_cast<double>(negativeCount) / tokens.size();
        score.neutrality = 1.0 - score.positivity - score.negativity;
        
        if (score.overall > 0.2) {
            score.category = "positive";
        } else if (score.overall < -0.2) {
            score.category = "negative";
        } else {
            score.category = "neutral";
        }
    }
    
    return score;
}

double SentimentAnalyzer::getWordSentiment(const std::string& word) {
    auto it = wordSentiments.find(NLPUtils::toLowerCase(word));
    return (it != wordSentiments.end()) ? it->second : 0.0;
}

void SentimentAnalyzer::updateWordSentiment(const std::string& word, double sentiment) {
    wordSentiments[NLPUtils::toLowerCase(word)] = sentiment;
}

// RecommendationEngine implementation
RecommendationEngine::RecommendationEngine() {
    NLPUtils::initializeWordLists();
}

double RecommendationEngine::calculateContentSimilarity(const std::shared_ptr<Proposal>& proposal1, 
                                                       const std::shared_ptr<Proposal>& proposal2) {
    std::string content1 = proposal1->getTitle() + " " + proposal1->getDescription();
    std::string content2 = proposal2->getTitle() + " " + proposal2->getDescription();
    
    return NLPUtils::calculateSimilarity(content1, content2);
}

double RecommendationEngine::calculateUserSimilarity(const std::string& userId1, const std::string& userId2) {
    auto& profile1 = getUserProfile(userId1);
    auto& profile2 = getUserProfile(userId2);
    
    // Calculate similarity based on voting history overlap
    std::set<std::string> votes1(profile1.votingHistory.begin(), profile1.votingHistory.end());
    std::set<std::string> votes2(profile2.votingHistory.begin(), profile2.votingHistory.end());
    
    std::set<std::string> intersection;
    std::set_intersection(votes1.begin(), votes1.end(), votes2.begin(), votes2.end(),
                         std::inserter(intersection, intersection.begin()));
    
    std::set<std::string> unionSet;
    std::set_union(votes1.begin(), votes1.end(), votes2.begin(), votes2.end(),
                   std::inserter(unionSet, unionSet.begin()));
    
    if (unionSet.empty()) return 0.0;
    
    return static_cast<double>(intersection.size()) / unionSet.size();
}

std::vector<std::string> RecommendationEngine::findSimilarUsers(const std::string& userId, int maxUsers) {
    std::vector<std::pair<std::string, double>> similarities;
    
    for (const auto& pair : userProfiles) {
        if (pair.first != userId) {
            double similarity = calculateUserSimilarity(userId, pair.first);
            similarities.emplace_back(pair.first, similarity);
        }
    }
    
    std::sort(similarities.begin(), similarities.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::vector<std::string> similarUsers;
    for (int i = 0; i < std::min(maxUsers, static_cast<int>(similarities.size())); ++i) {
        similarUsers.push_back(similarities[i].first);
    }
    
    return similarUsers;
}

void RecommendationEngine::updateUserProfile(const std::string& userId, const std::shared_ptr<User>& user) {
    auto& profile = getUserProfile(userId);
    profile.votingHistory = user->getVotedProposals();
    profile.activityLevel = std::min(1.0, profile.votingHistory.size() / 10.0);  // Normalize to 0-1
}

UserProfile& RecommendationEngine::getUserProfile(const std::string& userId) {
    if (userProfiles.find(userId) == userProfiles.end()) {
        userProfiles[userId] = UserProfile(userId);
    }
    return userProfiles[userId];
}

std::vector<RecommendationResult> RecommendationEngine::getPersonalizedRecommendations(
    const std::string& userId, 
    const std::vector<std::shared_ptr<Proposal>>& allProposals,
    int maxRecommendations) {
    
    auto contentBased = getContentBasedRecommendations(userId, allProposals, maxRecommendations / 2);
    auto collaborative = getCollaborativeRecommendations(userId, allProposals, maxRecommendations / 2);
    
    std::vector<RecommendationResult> combined;
    combined.insert(combined.end(), contentBased.begin(), contentBased.end());
    combined.insert(combined.end(), collaborative.begin(), collaborative.end());
    
    std::sort(combined.begin(), combined.end(), std::greater<RecommendationResult>());
    
    if (combined.size() > maxRecommendations) {
        combined.resize(maxRecommendations);
    }
    
    return combined;
}

std::vector<RecommendationResult> RecommendationEngine::getContentBasedRecommendations(
    const std::string& userId,
    const std::vector<std::shared_ptr<Proposal>>& allProposals,
    int maxRecommendations) {
    
    std::vector<RecommendationResult> recommendations;
    auto& profile = getUserProfile(userId);
    
    // Find proposals similar to ones the user has voted for
    for (const auto& proposal : allProposals) {
        if (std::find(profile.votingHistory.begin(), profile.votingHistory.end(), 
                     proposal->getProposalId()) != profile.votingHistory.end()) {
            continue;  // Skip proposals user has already voted for
        }
        
        double maxSimilarity = 0.0;
        std::string similarProposalId;
        
        // Find the most similar proposal the user has voted for
        for (const auto& votedProposalId : profile.votingHistory) {
            for (const auto& votedProposal : allProposals) {
                if (votedProposal->getProposalId() == votedProposalId) {
                    double similarity = calculateContentSimilarity(proposal, votedProposal);
                    if (similarity > maxSimilarity) {
                        maxSimilarity = similarity;
                        similarProposalId = votedProposalId;
                    }
                    break;
                }
            }
        }
        
        if (maxSimilarity > 0.3) {  // Threshold for similarity
            std::string reason = "Similar to proposal " + similarProposalId + " you voted for";
            recommendations.emplace_back(proposal->getProposalId(), maxSimilarity, reason);
        }
    }
    
    std::sort(recommendations.begin(), recommendations.end(), std::greater<RecommendationResult>());
    
    if (recommendations.size() > maxRecommendations) {
        recommendations.resize(maxRecommendations);
    }
    
    return recommendations;
}

std::vector<RecommendationResult> RecommendationEngine::getCollaborativeRecommendations(
    const std::string& userId,
    const std::vector<std::shared_ptr<Proposal>>& allProposals,
    int maxRecommendations) {
    
    std::vector<RecommendationResult> recommendations;
    auto& profile = getUserProfile(userId);
    auto similarUsers = findSimilarUsers(userId, 5);
    
    std::unordered_map<std::string, int> proposalVotes;
    
    // Count votes from similar users
    for (const auto& similarUserId : similarUsers) {
        auto& similarProfile = getUserProfile(similarUserId);
        for (const auto& proposalId : similarProfile.votingHistory) {
            if (std::find(profile.votingHistory.begin(), profile.votingHistory.end(), 
                         proposalId) == profile.votingHistory.end()) {
                proposalVotes[proposalId]++;
            }
        }
    }
    
    // Convert to recommendations
    for (const auto& pair : proposalVotes) {
        double score = static_cast<double>(pair.second) / similarUsers.size();
        std::string reason = "Liked by " + std::to_string(pair.second) + " similar users";
        recommendations.emplace_back(pair.first, score, reason);
    }
    
    std::sort(recommendations.begin(), recommendations.end(), std::greater<RecommendationResult>());
    
    if (recommendations.size() > maxRecommendations) {
        recommendations.resize(maxRecommendations);
    }
    
    return recommendations;
}

// AnomalyDetector implementation
AnomalyDetector::AnomalyDetector() {}

void AnomalyDetector::recordVote(const std::string& userId, const std::string& proposalId, const std::string& timestamp) {
    userVotingPatterns[userId].push_back(proposalId + ":" + timestamp);
    userActivityScores[userId] += 1.0;
}

double AnomalyDetector::calculateVotingVelocity(const std::string& userId) {
    auto it = userVotingPatterns.find(userId);
    if (it == userVotingPatterns.end() || it->second.size() < 2) {
        return 0.0;
    }
    
    // Simple velocity calculation based on recent votes
    return static_cast<double>(it->second.size()) / 24.0;  // votes per hour (assuming 24-hour period)
}

bool AnomalyDetector::detectBotBehavior(const std::string& userId) {
    double velocity = calculateVotingVelocity(userId);
    return velocity > 10.0;  // More than 10 votes per hour is suspicious
}

std::vector<std::string> AnomalyDetector::detectCollusionGroups() {
    std::vector<std::string> collusionGroups;
    
    // Simple collusion detection: users who vote on exactly the same proposals
    for (auto it1 = userVotingPatterns.begin(); it1 != userVotingPatterns.end(); ++it1) {
        for (auto it2 = std::next(it1); it2 != userVotingPatterns.end(); ++it2) {
            if (it1->second == it2->second && it1->second.size() > 3) {
                collusionGroups.push_back("Potential collusion: " + it1->first + " and " + it2->first);
            }
        }
    }
    
    return collusionGroups;
}

std::vector<AnomalyResult> AnomalyDetector::detectAnomalies() {
    std::vector<AnomalyResult> anomalies;
    
    // Detect bot behavior
    for (const auto& pair : userVotingPatterns) {
        if (detectBotBehavior(pair.first)) {
            double velocity = calculateVotingVelocity(pair.first);
            anomalies.emplace_back("bot_behavior", pair.first, std::min(1.0, velocity / 20.0),
                                 "Unusually high voting velocity: " + std::to_string(velocity) + " votes/hour");
        }
    }
    
    // Detect collusion
    auto collusionGroups = detectCollusionGroups();
    for (const auto& group : collusionGroups) {
        anomalies.emplace_back("collusion", "multiple", 0.8, group);
    }
    
    return anomalies;
}

double AnomalyDetector::calculateUserCredibility(const std::string& userId) {
    if (isUserSuspicious(userId)) {
        return 0.2;  // Low credibility for suspicious users
    }
    
    auto it = userActivityScores.find(userId);
    if (it == userActivityScores.end()) {
        return 0.5;  // Default credibility
    }
    
    // Higher activity generally means higher credibility, but cap it
    return std::min(1.0, 0.5 + (it->second / 20.0));
}

bool AnomalyDetector::isUserSuspicious(const std::string& userId) {
    return detectBotBehavior(userId);
}

// PredictiveAnalytics implementation
PredictiveAnalytics::PredictiveAnalytics() {}

void PredictiveAnalytics::updateVotingTrend(const std::string& proposalId, int currentVotes) {
    auto& trend = proposalTrends[proposalId];
    trend.proposalId = proposalId;
    
    // Simple trend tracking (in a real system, you'd track hourly data)
    if (trend.hourlyVotes.empty()) {
        trend.hourlyVotes.push_back(currentVotes);
    } else {
        int lastVotes = trend.hourlyVotes.back();
        trend.hourlyVotes.push_back(currentVotes);
        trend.momentum = currentVotes - lastVotes;
    }
    
    // Keep only recent data (last 24 hours)
    if (trend.hourlyVotes.size() > 24) {
        trend.hourlyVotes.erase(trend.hourlyVotes.begin());
    }
}

double PredictiveAnalytics::predictFinalVoteCount(const std::string& proposalId) {
    auto it = proposalTrends.find(proposalId);
    if (it == proposalTrends.end() || it->second.hourlyVotes.empty()) {
        return 0.0;
    }
    
    auto& trend = it->second;
    int currentVotes = trend.hourlyVotes.back();
    
    // Simple linear prediction based on momentum
    double predictedIncrease = trend.momentum * 24;  // Predict for next 24 hours
    return currentVotes + std::max(0.0, predictedIncrease);
}

std::vector<std::string> PredictiveAnalytics::predictTopProposals(int count) {
    std::vector<std::pair<std::string, double>> predictions;
    
    for (const auto& pair : proposalTrends) {
        double predicted = predictFinalVoteCount(pair.first);
        predictions.emplace_back(pair.first, predicted);
    }
    
    std::sort(predictions.begin(), predictions.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::vector<std::string> topProposals;
    for (int i = 0; i < std::min(count, static_cast<int>(predictions.size())); ++i) {
        topProposals.push_back(predictions[i].first);
    }
    
    return topProposals;
}

double PredictiveAnalytics::calculateMomentum(const std::string& proposalId) {
    auto it = proposalTrends.find(proposalId);
    return (it != proposalTrends.end()) ? it->second.momentum : 0.0;
}

// IntelligenceEngine implementation
IntelligenceEngine::IntelligenceEngine(VotingSystem* vs) : votingSystem(vs) {}

std::vector<RecommendationResult> IntelligenceEngine::getRecommendationsForUser(const std::string& userId, int maxResults) {
    if (!votingSystem) return {};
    
    // Get all proposals from the voting system
    auto allProposals = votingSystem->getAllProposals();
    
    return recommendationEngine.getPersonalizedRecommendations(userId, allProposals, maxResults);
}

SentimentScore IntelligenceEngine::analyzeProposalSentiment(const std::string& proposalId) {
    if (!votingSystem) return SentimentScore();
    
    auto proposal = votingSystem->getProposal(proposalId);
    if (!proposal) return SentimentScore();
    
    std::string text = proposal->getTitle() + " " + proposal->getDescription();
    return sentimentAnalyzer.analyzeSentiment(text);
}

std::vector<std::pair<std::string, SentimentScore>> IntelligenceEngine::analyzeBatchSentiments(const std::vector<std::string>& proposalIds) {
    std::vector<std::pair<std::string, SentimentScore>> results;
    
    for (const auto& proposalId : proposalIds) {
        SentimentScore score = analyzeProposalSentiment(proposalId);
        results.emplace_back(proposalId, score);
    }
    
    return results;
}

std::vector<AnomalyResult> IntelligenceEngine::performSecurityScan() {
    return anomalyDetector.detectAnomalies();
}

bool IntelligenceEngine::validateVote(const std::string& userId, const std::string& proposalId) {
    return !anomalyDetector.isUserSuspicious(userId);
}

std::vector<std::pair<std::string, double>> IntelligenceEngine::getPredictedRankings() {
    auto topProposals = predictiveAnalytics.predictTopProposals(10);
    std::vector<std::pair<std::string, double>> rankings;
    
    for (const auto& proposalId : topProposals) {
        double predicted = predictiveAnalytics.predictFinalVoteCount(proposalId);
        rankings.emplace_back(proposalId, predicted);
    }
    
    return rankings;
}

double IntelligenceEngine::getProposalMomentum(const std::string& proposalId) {
    return predictiveAnalytics.calculateMomentum(proposalId);
}

void IntelligenceEngine::learnFromVote(const std::string& userId, const std::string& proposalId) {
    // Update user profile
    if (votingSystem) {
        auto user = votingSystem->getUser(userId);
        if (user) {
            recommendationEngine.updateUserProfile(userId, user);
        }
        
        // Record vote for anomaly detection
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::string timestamp = std::to_string(time_t);
        anomalyDetector.recordVote(userId, proposalId, timestamp);
        
        // Update voting trends
        auto proposal = votingSystem->getProposal(proposalId);
        if (proposal) {
            predictiveAnalytics.updateVotingTrend(proposalId, proposal->getVoteCount());
        }
    }
}

void IntelligenceEngine::updateIntelligence() {
    // Perform periodic updates and learning
    // This could include retraining models, updating user profiles, etc.
}

std::string IntelligenceEngine::generateInsightReport() {
    std::stringstream report;
    report << "\n=== INTELLIGENCE INSIGHTS REPORT ===\n";
    
    // Security insights
    auto anomalies = performSecurityScan();
    report << "Security Status: " << (anomalies.empty() ? "SECURE" : "ALERTS DETECTED") << "\n";
    report << "Anomalies Detected: " << anomalies.size() << "\n";
    
    // Prediction insights
    auto predictions = getPredictedRankings();
    report << "Top Predicted Proposals: " << predictions.size() << "\n";
    
    // Sentiment insights
    report << "Sentiment Analysis: Active\n";
    report << "Recommendation Engine: Active\n";
    
    return report.str();
}

std::vector<std::string> IntelligenceEngine::getSystemRecommendations() {
    std::vector<std::string> recommendations;
    
    auto anomalies = performSecurityScan();
    if (!anomalies.empty()) {
        recommendations.push_back("Consider implementing additional security measures");
    }
    
    recommendations.push_back("System is operating with full intelligence capabilities");
    recommendations.push_back("All AI modules are active and learning");
    
    return recommendations;
}
