#ifndef CONSISTENCY_SCORER_H
#define CONSISTENCY_SCORER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cmath>
#include <algorithm>
#include <numeric>

// Forward declarations
class Proposal;
class User;

/**
 * Structure to hold user's proposal similarity history
 */
struct ProposalSimilarityRecord {
    std::string proposalId;
    double similarityScore;
    std::string timestamp;
    std::string topicId;
    
    ProposalSimilarityRecord() : similarityScore(0.0) {}
    ProposalSimilarityRecord(const std::string& pid, double sim, const std::string& ts, const std::string& tid)
        : proposalId(pid), similarityScore(sim), timestamp(ts), topicId(tid) {}
};

/**
 * Structure to hold consistency metrics for a user
 */
struct ConsistencyMetrics {
    std::string userId;
    double meanSimilarity;          // μ_i: mean similarity across all proposals
    double stdDevSimilarity;        // σ_i: standard deviation of similarities
    double consistencyScore;        // 1 / (1 + σ_i): bounded in (0, 1]
    int proposalCount;              // number of proposals analyzed
    double variance;                // σ_i²
    
    ConsistencyMetrics() 
        : meanSimilarity(0.0), stdDevSimilarity(0.0), 
          consistencyScore(0.5), proposalCount(0), variance(0.0) {}
    
    ConsistencyMetrics(const std::string& uid)
        : userId(uid), meanSimilarity(0.0), stdDevSimilarity(0.0),
          consistencyScore(0.5), proposalCount(0), variance(0.0) {}
};

/**
 * Structure for weighted ranking score
 */
struct WeightedRankingScore {
    std::string proposalId;
    std::string userId;
    double relevanceScore;          // Base relevance to topic
    double consistencyScore;        // User's consistency score
    double trustScore;              // User's trust/credibility score
    double recencyScore;            // Time-based recency factor
    double finalWeightedScore;      // Combined weighted score
    
    // Configurable weights (default from documentation)
    static double alpha;  // relevance weight (0.55)
    static double beta;   // consistency weight (0.25)
    static double gamma;  // trust weight (0.15)
    static double delta;  // recency weight (0.05)
    
    WeightedRankingScore()
        : relevanceScore(0.0), consistencyScore(0.5), 
          trustScore(0.5), recencyScore(1.0), finalWeightedScore(0.0) {}
    
    // Calculate final weighted score using formula:
    // final = α*relevance + β*consistency + γ*trust + δ*recency
    void calculateFinalScore() {
        finalWeightedScore = alpha * relevanceScore +
                           beta * consistencyScore +
                           gamma * trustScore +
                           delta * recencyScore;
    }
    
    bool operator<(const WeightedRankingScore& other) const {
        return finalWeightedScore < other.finalWeightedScore;
    }
};

/**
 * ConsistencyScorer: Implements weight-based consistency and stability scoring
 * 
 * Key Concept:
 * - Local proximity: How close a single proposal is to core/subtopics
 * - Global consistency: How stable that user's proposals are across all subtopics over time
 * 
 * Users with consistent, stable contributions get rewarded with consistency bonus
 * Users with erratic, inconsistent proposals get penalized
 */
class ConsistencyScorer {
private:
    // User ID -> list of their proposal similarity records
    std::unordered_map<std::string, std::vector<ProposalSimilarityRecord>> userProposalHistory;
    
    // User ID -> current consistency metrics
    std::unordered_map<std::string, ConsistencyMetrics> userConsistencyCache;
    
    // Configuration
    int rollingWindowSize;              // Maximum proposals to keep in rolling window
    double newUserDefaultConsistency;   // Default consistency for new users (0.5)
    bool useRollingWindow;              // Whether to use rolling window or all history
    
    // Helper methods
    double calculateMean(const std::vector<double>& values) const;
    double calculateStdDev(const std::vector<double>& values, double mean) const;
    double calculateVariance(const std::vector<double>& values, double mean) const;
    
public:
    /**
     * Constructor
     * @param windowSize Maximum number of recent proposals to consider (default: 50)
     * @param useWindow Whether to use rolling window or all history (default: true)
     */
    ConsistencyScorer(int windowSize = 50, bool useWindow = true);
    
    /**
     * Record a proposal's similarity score for a user
     * @param userId User who created the proposal
     * @param proposalId Proposal identifier
     * @param similarityScore Similarity score to core topic (0.0 - 1.0)
     * @param timestamp When the proposal was created
     * @param topicId Topic identifier
     */
    void recordProposalSimilarity(const std::string& userId,
                                  const std::string& proposalId,
                                  double similarityScore,
                                  const std::string& timestamp,
                                  const std::string& topicId = "");
    
    /**
     * Calculate consistency metrics for a user
     * Computes mean, std dev, variance, and consistency score
     * @param userId User identifier
     * @return ConsistencyMetrics structure with calculated values
     */
    ConsistencyMetrics calculateConsistencyMetrics(const std::string& userId);
    
    /**
     * Get cached consistency score for a user
     * Returns default (0.5) if user not found or has no history
     * @param userId User identifier
     * @return Consistency score in range (0, 1]
     */
    double getUserConsistencyScore(const std::string& userId);
    
    /**
     * Get full consistency metrics for a user
     * @param userId User identifier
     * @return ConsistencyMetrics structure
     */
    ConsistencyMetrics getUserConsistencyMetrics(const std::string& userId);
    
    /**
     * Update consistency metrics for a specific user
     * Call this after recording new proposal similarities
     * @param userId User identifier
     */
    void updateUserConsistency(const std::string& userId);
    
    /**
     * Update consistency metrics for all users
     * Call this periodically (e.g., hourly or when topic updates)
     */
    void updateAllConsistencyScores();
    
    /**
     * Calculate weighted ranking score for a proposal
     * Combines relevance, consistency, trust, and recency
     * @param proposalId Proposal identifier
     * @param userId User who created the proposal
     * @param relevanceScore Base relevance to topic (0.0 - 1.0)
     * @param trustScore User's trust/credibility score (0.0 - 1.0)
     * @param recencyScore Time-based recency factor (0.0 - 1.0)
     * @return WeightedRankingScore structure with final score
     */
    WeightedRankingScore calculateWeightedScore(const std::string& proposalId,
                                               const std::string& userId,
                                               double relevanceScore,
                                               double trustScore = 0.5,
                                               double recencyScore = 1.0);
    
    /**
     * Get proposal count for a user
     * @param userId User identifier
     * @return Number of proposals in history
     */
    int getUserProposalCount(const std::string& userId) const;
    
    /**
     * Clear history for a specific user
     * @param userId User identifier
     */
    void clearUserHistory(const std::string& userId);
    
    /**
     * Clear all history
     */
    void clearAllHistory();
    
    /**
     * Get all users with consistency scores
     * @return Vector of user IDs
     */
    std::vector<std::string> getAllTrackedUsers() const;
    
    /**
     * Get users with high consistency (> threshold)
     * @param threshold Minimum consistency score (default: 0.7)
     * @return Vector of user IDs
     */
    std::vector<std::string> getHighConsistencyUsers(double threshold = 0.7) const;
    
    /**
     * Get users with low consistency (< threshold)
     * Potentially indicates erratic behavior
     * @param threshold Maximum consistency score (default: 0.3)
     * @return Vector of user IDs
     */
    std::vector<std::string> getLowConsistencyUsers(double threshold = 0.3) const;
    
    /**
     * Get statistics about consistency scores
     * @return String with formatted statistics
     */
    std::string getConsistencyStatistics() const;
    
    /**
     * Configure scoring weights
     * @param alphaRelevance Weight for relevance score (default: 0.55)
     * @param betaConsistency Weight for consistency score (default: 0.25)
     * @param gammaTrust Weight for trust score (default: 0.15)
     * @param deltaRecency Weight for recency score (default: 0.05)
     */
    static void configureWeights(double alphaRelevance,
                                 double betaConsistency,
                                 double gammaTrust,
                                 double deltaRecency);
    
    /**
     * Get current weight configuration
     * @return String with current weights
     */
    static std::string getWeightConfiguration();
    
    /**
     * Set rolling window size
     * @param windowSize New window size
     */
    void setRollingWindowSize(int windowSize);
    
    /**
     * Set whether to use rolling window
     * @param useWindow True to use rolling window, false for all history
     */
    void setUseRollingWindow(bool useWindow);
};

#endif // CONSISTENCY_SCORER_H
