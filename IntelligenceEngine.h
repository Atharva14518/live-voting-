#ifndef INTELLIGENCE_ENGINE_H
#define INTELLIGENCE_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <cmath>
#include <set>
#include <sstream>
#include <regex>

// Forward declarations
class User;
class Proposal;
class VotingSystem;

// Sentiment analysis result
struct SentimentScore {
    double positivity;      // 0.0 to 1.0
    double negativity;      // 0.0 to 1.0
    double neutrality;      // 0.0 to 1.0
    double overall;         // -1.0 (very negative) to 1.0 (very positive)
    std::string category;   // "positive", "negative", "neutral"
    
    SentimentScore() : positivity(0.0), negativity(0.0), neutrality(1.0), overall(0.0), category("neutral") {}
};

// Recommendation result
struct RecommendationResult {
    std::string proposalId;
    double score;           // 0.0 to 1.0 confidence
    std::string reason;     // Why this was recommended
    
    RecommendationResult() : score(0.0) {}
    RecommendationResult(const std::string& id, double s, const std::string& r) 
        : proposalId(id), score(s), reason(r) {}
    
    bool operator>(const RecommendationResult& other) const {
        return score > other.score;
    }
};

// Anomaly detection result
struct AnomalyResult {
    std::string type;       // "suspicious_voting", "bot_behavior", "collusion"
    std::string userId;
    double severity;        // 0.0 to 1.0
    std::string description;
    std::string timestamp;
    
    AnomalyResult() : severity(0.0) {}
    AnomalyResult(const std::string& t, const std::string& u, double s, const std::string& d)
        : type(t), userId(u), severity(s), description(d) {}
};

// User behavior profile
struct UserProfile {
    std::string userId;
    std::vector<std::string> preferredCategories;
    std::unordered_map<std::string, double> topicInterests;  // topic -> interest score
    double activityLevel;       // 0.0 to 1.0
    double credibilityScore;    // 0.0 to 1.0
    std::vector<std::string> votingHistory;
    
    UserProfile() : activityLevel(0.0), credibilityScore(0.5) {}
    UserProfile(const std::string& id) : userId(id), activityLevel(0.0), credibilityScore(0.5) {}
};

// Natural Language Processing utilities
class NLPUtils {
public:
    static std::vector<std::string> positiveWords;
    static std::vector<std::string> negativeWords;
    static std::vector<std::string> stopWords;
    static void initializeWordLists();
    static std::vector<std::string> tokenize(const std::string& text);
    static std::vector<std::string> removeStopWords(const std::vector<std::string>& tokens);
    static std::string toLowerCase(const std::string& text);
    static double calculateSimilarity(const std::string& text1, const std::string& text2);
    static std::vector<std::string> extractKeywords(const std::string& text, int maxKeywords = 5);
};

// Sentiment Analysis Engine
class SentimentAnalyzer {
private:
    std::unordered_map<std::string, double> wordSentiments;
    
public:
    SentimentAnalyzer();
    SentimentScore analyzeSentiment(const std::string& text);
    double getWordSentiment(const std::string& word);
    void updateWordSentiment(const std::string& word, double sentiment);
};

// Recommendation Engine
class RecommendationEngine {
private:
    std::unordered_map<std::string, UserProfile> userProfiles;
    SentimentAnalyzer sentimentAnalyzer;
    
    double calculateContentSimilarity(const std::shared_ptr<Proposal>& proposal1, 
                                    const std::shared_ptr<Proposal>& proposal2);
    double calculateUserSimilarity(const std::string& userId1, const std::string& userId2);
    std::vector<std::string> findSimilarUsers(const std::string& userId, int maxUsers = 5);
    
public:
    RecommendationEngine();
    
    // Profile management
    void updateUserProfile(const std::string& userId, const std::shared_ptr<User>& user);
    UserProfile& getUserProfile(const std::string& userId);
    
    // Recommendation methods
    std::vector<RecommendationResult> getPersonalizedRecommendations(
        const std::string& userId, 
        const std::vector<std::shared_ptr<Proposal>>& allProposals,
        int maxRecommendations = 5
    );
    
    std::vector<RecommendationResult> getContentBasedRecommendations(
        const std::string& userId,
        const std::vector<std::shared_ptr<Proposal>>& allProposals,
        int maxRecommendations = 5
    );
    
    std::vector<RecommendationResult> getCollaborativeRecommendations(
        const std::string& userId,
        const std::vector<std::shared_ptr<Proposal>>& allProposals,
        int maxRecommendations = 5
    );
};

// Anomaly Detection Engine
class AnomalyDetector {
private:
    std::unordered_map<std::string, std::vector<std::string>> userVotingPatterns;
    std::unordered_map<std::string, double> userActivityScores;
    
    double calculateVotingVelocity(const std::string& userId);
    bool detectBotBehavior(const std::string& userId);
    std::vector<std::string> detectCollusionGroups();
    
public:
    AnomalyDetector();
    
    void recordVote(const std::string& userId, const std::string& proposalId, const std::string& timestamp);
    std::vector<AnomalyResult> detectAnomalies();
    double calculateUserCredibility(const std::string& userId);
    bool isUserSuspicious(const std::string& userId);
};

// Predictive Analytics Engine
class PredictiveAnalytics {
private:
    struct VotingTrend {
        std::string proposalId;
        std::vector<int> hourlyVotes;  // votes per hour
        double momentum;               // rate of change
        double predictedFinalVotes;
    };
    
    std::unordered_map<std::string, VotingTrend> proposalTrends;
    
public:
    PredictiveAnalytics();
    
    void updateVotingTrend(const std::string& proposalId, int currentVotes);
    double predictFinalVoteCount(const std::string& proposalId);
    std::vector<std::string> predictTopProposals(int count = 5);
    double calculateMomentum(const std::string& proposalId);
};

// Main Intelligence Engine
class IntelligenceEngine {
private:
    RecommendationEngine recommendationEngine;
    AnomalyDetector anomalyDetector;
    SentimentAnalyzer sentimentAnalyzer;
    PredictiveAnalytics predictiveAnalytics;
    
    VotingSystem* votingSystem;  // Reference to main voting system
    
public:
    IntelligenceEngine(VotingSystem* vs);
    
    // Recommendation services
    std::vector<RecommendationResult> getRecommendationsForUser(const std::string& userId, int maxResults = 5);
    
    // Sentiment analysis services
    SentimentScore analyzeProposalSentiment(const std::string& proposalId);
    std::vector<std::pair<std::string, SentimentScore>> analyzeBatchSentiments(const std::vector<std::string>& proposalIds);
    
    // Anomaly detection services
    std::vector<AnomalyResult> performSecurityScan();
    bool validateVote(const std::string& userId, const std::string& proposalId);
    
    // Predictive analytics services
    std::vector<std::pair<std::string, double>> getPredictedRankings();
    double getProposalMomentum(const std::string& proposalId);
    
    // Learning and adaptation
    void learnFromVote(const std::string& userId, const std::string& proposalId);
    void updateIntelligence();
    
    // Statistics and insights
    std::string generateInsightReport();
    std::vector<std::string> getSystemRecommendations();  // Recommendations for system improvement
};

#endif // INTELLIGENCE_ENGINE_H
