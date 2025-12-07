#ifndef ADVANCED_ANALYTICS_H
#define ADVANCED_ANALYTICS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <memory>
#include <algorithm>
#include <cmath>
#include <queue>
#include <numeric>

// Forward declarations
class Proposal;

// ==================== NORMALIZATION UTILITIES ====================

class NormalizationUtils {
public:
    // MinMax Normalization: scales values to [0, 1]
    static std::vector<double> minMaxNormalize(const std::vector<double>& values);
    
    // Z-Score Standardization: scales to mean=0, std=1
    static std::vector<double> zScoreStandardize(const std::vector<double>& values);
    
    // Calculate mean
    static double calculateMean(const std::vector<double>& values);
    
    // Calculate standard deviation
    static double calculateStdDev(const std::vector<double>& values, double mean);
};

// ==================== ENHANCED SIMILARITY METRICS ====================

class SimilarityMetrics {
public:
    // Jaccard Index: |A ∩ B| / |A ∪ B|
    static double jaccardSimilarity(const std::set<std::string>& set1, const std::set<std::string>& set2);
    
    // Cosine Similarity: (A · B) / (||A|| × ||B||)
    static double cosineSimilarity(const std::unordered_map<std::string, double>& vec1,
                                   const std::unordered_map<std::string, double>& vec2);
    
    // Combined similarity with weights
    static double combinedSimilarity(const std::string& text1, const std::string& text2,
                                    double jaccardWeight = 0.5, double cosineWeight = 0.5);
    
    // Create TF-IDF vector for text
    static std::unordered_map<std::string, double> createTFIDFVector(
        const std::string& text,
        const std::vector<std::string>& corpus);
};

// ==================== TOPIC ANALYSIS ====================

struct Topic {
    std::string topicId;
    std::string name;
    std::vector<std::string> keywords;  // Sorted for binary search
    std::unordered_map<std::string, double> keywordWeights;
    std::vector<std::string> subTopics;
    double relevanceScore;
    
    Topic() : relevanceScore(0.0) {}
    Topic(const std::string& id, const std::string& n) 
        : topicId(id), name(n), relevanceScore(0.0) {}
};

struct DecisionRanking {
    std::string proposalId;
    std::string title;
    double weightedRelevance;
    double timeScore;
    double priorityScore;
    double combinedScore;
    int rank;
    double percentile;
    std::vector<std::string> matchedTopics;
    
    DecisionRanking() : weightedRelevance(0.0), timeScore(0.0), 
                        priorityScore(0.0), combinedScore(0.0), 
                        rank(0), percentile(0.0) {}
    
    bool operator<(const DecisionRanking& other) const {
        return combinedScore < other.combinedScore;  // For max heap
    }
};

class TopicAnalyzer {
private:
    std::unordered_map<std::string, Topic> topics;
    std::unordered_map<std::string, std::vector<std::string>> proposalTopics;
    
    // Binary search on sorted keyword vector
    bool keywordExists(const std::vector<std::string>& sortedKeywords, 
                      const std::string& keyword) const;
    
    // Extract and decompose topics from text
    std::vector<std::string> extractTopicsFromText(const std::string& text);
    
    // Calculate topic relevance using similarity metrics
    double calculateTopicRelevance(const std::string& proposalText, 
                                   const Topic& topic);

public:
    TopicAnalyzer();
    
    // Topic management
    void addTopic(const std::string& topicId, const std::string& name, 
                 const std::vector<std::string>& keywords);
    
    void decomposeTopicIntoSubTopics(const std::string& topicId, 
                                     const std::vector<std::string>& subTopics);
    
    // Analyze proposal and assign topics
    void analyzeProposal(const std::shared_ptr<Proposal>& proposal);
    
    // Get topics for a proposal
    std::vector<std::string> getProposalTopics(const std::string& proposalId) const;
    
    // Get all proposals for a topic
    std::vector<std::string> getProposalsForTopic(const std::string& topicId) const;
    
    // Search for keyword in topic (binary search)
    bool searchKeywordInTopic(const std::string& topicId, const std::string& keyword) const;
    
    // Get topic information
    Topic getTopic(const std::string& topicId) const;
    
    // Get all topics
    std::vector<Topic> getAllTopics() const;
};

// ==================== LOGISTIC REGRESSION CLASSIFIER ====================

struct ClassificationLabel {
    std::string label;
    double confidence;
    std::vector<std::string> features;
    
    ClassificationLabel() : confidence(0.0) {}
    ClassificationLabel(const std::string& l, double c) 
        : label(l), confidence(c) {}
};

class LogisticRegressionClassifier {
private:
    std::unordered_map<std::string, std::vector<double>> weights;  // label -> feature weights
    std::vector<std::string> featureNames;
    double learningRate;
    int maxIterations;
    
    // Sigmoid function
    double sigmoid(double z) const;
    
    // Extract features from proposal
    std::vector<double> extractFeatures(const std::shared_ptr<Proposal>& proposal);
    
    // Calculate probability for a label
    double calculateProbability(const std::vector<double>& features, 
                               const std::string& label) const;

public:
    LogisticRegressionClassifier(double lr = 0.01, int maxIter = 1000);
    
    // Train the classifier
    void train(const std::vector<std::shared_ptr<Proposal>>& proposals,
              const std::vector<std::string>& labels);
    
    // Classify a proposal
    ClassificationLabel classify(const std::shared_ptr<Proposal>& proposal);
    
    // Get classification probabilities for all labels
    std::unordered_map<std::string, double> getClassProbabilities(
        const std::shared_ptr<Proposal>& proposal);
    
    // Add custom feature
    void addFeature(const std::string& featureName);
};

// ==================== TIME-BASED FILTERING ====================

struct TimeFilter {
    std::string filterType;  // "recent", "trending", "historical"
    int timeWindowHours;
    double decayFactor;
    
    TimeFilter() : timeWindowHours(24), decayFactor(0.9) {}
    TimeFilter(const std::string& type, int hours, double decay)
        : filterType(type), timeWindowHours(hours), decayFactor(decay) {}
};

class TimeBasedFilter {
private:
    std::unordered_map<std::string, std::chrono::system_clock::time_point> proposalTimestamps;
    
    // Calculate time decay score
    double calculateTimeDecay(const std::chrono::system_clock::time_point& timestamp,
                             double decayFactor) const;
    
    // Parse timestamp string
    std::chrono::system_clock::time_point parseTimestamp(const std::string& timestamp) const;

public:
    TimeBasedFilter();
    
    // Register proposal timestamp
    void registerProposal(const std::string& proposalId, const std::string& timestamp);
    
    // Apply time-based filtering
    std::vector<std::string> filterByTime(const std::vector<std::string>& proposalIds,
                                         const TimeFilter& filter);
    
    // Calculate time score for ranking
    double calculateTimeScore(const std::string& proposalId, const TimeFilter& filter);
    
    // Get recent proposals
    std::vector<std::string> getRecentProposals(int hours = 24);
    
    // Get trending proposals (high activity in recent time)
    std::vector<std::string> getTrendingProposals(int hours = 6);
};

// ==================== DECISION RANKING ENGINE ====================

class DecisionRankingEngine {
private:
    TopicAnalyzer topicAnalyzer;
    LogisticRegressionClassifier classifier;
    TimeBasedFilter timeFilter;
    
    // Priority queue for rankings (max heap)
    std::priority_queue<DecisionRanking> rankingQueue;
    
    // Similarity matrix for proposals
    std::unordered_map<std::string, std::unordered_map<std::string, double>> similarityMatrix;
    
    // Calculate weighted relevance score
    double calculateWeightedRelevance(const std::shared_ptr<Proposal>& proposal,
                                     const std::vector<std::string>& coreTopics);
    
    // Calculate priority score
    double calculatePriorityScore(const std::shared_ptr<Proposal>& proposal);
    
    // Build similarity matrix
    void buildSimilarityMatrix(const std::vector<std::shared_ptr<Proposal>>& proposals);
    
    // Calculate percentile
    double calculatePercentile(double score, const std::vector<double>& allScores);

public:
    DecisionRankingEngine();
    
    // Initialize with proposals
    void initialize(const std::vector<std::shared_ptr<Proposal>>& proposals);
    
    // Rank decisions based on core topic
    std::vector<DecisionRanking> rankDecisionsByTopic(
        const std::string& coreTopicId,
        const std::vector<std::shared_ptr<Proposal>>& proposals,
        const TimeFilter& timeFilter = TimeFilter());
    
    // Get top N decisions
    std::vector<DecisionRanking> getTopDecisions(int n = 10);
    
    // Get decision ranking for specific proposal
    DecisionRanking getProposalRanking(const std::string& proposalId);
    
    // Get similarity between two proposals
    double getProposalSimilarity(const std::string& proposalId1, 
                                const std::string& proposalId2);
    
    // Update rankings with new data
    void updateRankings(const std::vector<std::shared_ptr<Proposal>>& proposals);
    
    // Get rank and percentile statistics
    std::string getRankingStatistics();
    
    // Access sub-components
    TopicAnalyzer& getTopicAnalyzer() { return topicAnalyzer; }
    LogisticRegressionClassifier& getClassifier() { return classifier; }
    TimeBasedFilter& getTimeFilter() { return timeFilter; }
};

// ==================== RANK AND PERCENTILE SYSTEM ====================

class RankPercentileSystem {
private:
    std::vector<double> scores;
    std::unordered_map<std::string, int> proposalRanks;
    std::unordered_map<std::string, double> proposalPercentiles;
    
    // Calculate percentile rank
    double calculatePercentileRank(double score, const std::vector<double>& allScores);

public:
    RankPercentileSystem();
    
    // Update rankings
    void updateRankings(const std::vector<std::pair<std::string, double>>& proposalScores);
    
    // Get rank for proposal
    int getRank(const std::string& proposalId) const;
    
    // Get percentile for proposal
    double getPercentile(const std::string& proposalId) const;
    
    // Get proposals in percentile range
    std::vector<std::string> getProposalsInPercentileRange(double minPercentile, 
                                                           double maxPercentile);
    
    // Get top N percent proposals
    std::vector<std::string> getTopPercentProposals(double percentile);
    
    // Get statistics
    std::string getStatistics() const;
};

#endif // ADVANCED_ANALYTICS_H
