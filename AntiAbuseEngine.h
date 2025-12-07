#ifndef ANTI_ABUSE_ENGINE_H
#define ANTI_ABUSE_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <memory>
#include <chrono>
#include <queue>
#include <algorithm>

/**
 * Structure for vote event tracking
 */
struct VoteEvent {
    std::string voteId;
    std::string userId;
    std::string proposalId;
    std::chrono::system_clock::time_point timestamp;
    std::string ipHash;          // Hashed IP address for privacy
    std::string deviceHash;      // Hashed device fingerprint
    
    VoteEvent() {}
    VoteEvent(const std::string& vid, const std::string& uid, const std::string& pid,
              const std::chrono::system_clock::time_point& ts,
              const std::string& ip = "", const std::string& dev = "")
        : voteId(vid), userId(uid), proposalId(pid), timestamp(ts),
          ipHash(ip), deviceHash(dev) {}
};

/**
 * Structure for bot detection result
 */
struct BotDetectionResult {
    std::string userId;
    double botLikelihood;           // 0.0 to 1.0
    double votingVelocity;          // votes per minute
    double avgInterVoteGapMs;       // average milliseconds between votes
    int deviceDiversity;            // number of unique devices
    int ipDiversity;                // number of unique IPs
    bool isSuspicious;              // exceeds thresholds
    std::string reason;             // explanation
    
    BotDetectionResult()
        : botLikelihood(0.0), votingVelocity(0.0), avgInterVoteGapMs(0.0),
          deviceDiversity(0), ipDiversity(0), isSuspicious(false) {}
};

/**
 * Structure for collusion detection result
 */
struct CollusionDetectionResult {
    std::vector<std::string> userGroup;     // Group of users colluding
    double collusionScore;                   // 0.0 to 1.0
    int coVoteCount;                         // number of co-votes
    double coVoteRate;                       // co-votes / total possible
    std::string description;
    bool isSuspicious;
    
    CollusionDetectionResult()
        : collusionScore(0.0), coVoteCount(0), coVoteRate(0.0), isSuspicious(false) {}
};

/**
 * Structure for threat alert
 */
struct ThreatAlert {
    std::string alertId;
    std::string alertType;      // "bot_detected", "collusion_detected", "velocity_spike"
    double severity;            // 0.0 to 1.0
    std::vector<std::string> involvedUsers;
    std::string description;
    std::chrono::system_clock::time_point timestamp;
    bool resolved;
    
    ThreatAlert()
        : severity(0.0), resolved(false) {}
};

/**
 * User credibility scoring structure
 */
struct UserCredibilityScore {
    std::string userId;
    double trustScore;              // 0.0 to 1.0 (overall credibility)
    
    // Component scores
    double accountAgeScore;         // based on account age
    double deviceDiversityScore;    // multiple devices = suspicious
    double majorityAgreementScore;  // agreement with majority votes
    double verificationScore;       // account verification status
    double botLikelihood;           // from bot detection (inverse)
    double collusionScore;          // from collusion detection (inverse)
    double consistencyScore;        // vote pattern consistency
    double reportScore;             // history of being reported/flagged
    
    UserCredibilityScore()
        : trustScore(0.5), accountAgeScore(0.0), deviceDiversityScore(0.0),
          majorityAgreementScore(0.5), verificationScore(0.0),
          botLikelihood(0.0), collusionScore(0.0), 
          consistencyScore(0.5), reportScore(1.0) {}
};

/**
 * Sliding window for velocity tracking
 */
class SlidingWindow {
private:
    std::deque<std::chrono::system_clock::time_point> timestamps;
    std::chrono::seconds windowDuration;
    
public:
    SlidingWindow(int seconds = 60) 
        : windowDuration(seconds) {}
    
    void addEvent(const std::chrono::system_clock::time_point& timestamp);
    void cleanup(const std::chrono::system_clock::time_point& currentTime);
    int getEventCount() const { return timestamps.size(); }
    double getRate() const;  // events per minute
    double getAverageGapMs() const;
};

/**
 * Co-voting graph for collusion detection
 */
class CoVotingGraph {
private:
    // adjacency[user1][user2] = number of times they voted together
    std::unordered_map<std::string, std::unordered_map<std::string, int>> adjacency;
    
    // Store vote history per proposal for co-vote calculation
    std::unordered_map<std::string, std::unordered_set<std::string>> proposalVoters;
    
public:
    void addVote(const std::string& userId, const std::string& proposalId);
    int getCoVoteCount(const std::string& user1, const std::string& user2) const;
    std::vector<std::string> getNeighbors(const std::string& userId) const;
    int getEdgeCount() const;
    std::vector<std::vector<std::string>> detectCommunities(int minCoVotes = 5);
    void clear();
};

/**
 * AntiAbuseEngine: Comprehensive anti-abuse detection system
 * 
 * Features:
 * - Bot detection via velocity analysis
 * - Collusion detection via co-voting graph analysis
 * - User credibility scoring
 * - Real-time threat alerts
 */
class AntiAbuseEngine {
private:
    // Vote event storage
    std::unordered_map<std::string, std::vector<VoteEvent>> userVoteHistory;
    std::unordered_map<std::string, SlidingWindow> userVelocityWindows;
    
    // IP and device tracking
    std::unordered_map<std::string, std::unordered_set<std::string>> userIPs;
    std::unordered_map<std::string, std::unordered_set<std::string>> userDevices;
    std::unordered_map<std::string, std::vector<std::string>> ipToUsers;
    std::unordered_map<std::string, std::vector<std::string>> deviceToUsers;
    
    // Co-voting graph for collusion detection
    CoVotingGraph coVotingGraph;
    
    // Bot detection cache
    std::unordered_map<std::string, BotDetectionResult> botDetectionCache;
    
    // Collusion detection cache
    std::vector<CollusionDetectionResult> collusionDetectionCache;
    
    // User credibility scores
    std::unordered_map<std::string, UserCredibilityScore> userCredibilityScores;
    
    // Threat alerts
    std::vector<ThreatAlert> threatAlerts;
    std::unordered_map<std::string, bool> suspiciousUsers;
    
    // Configuration thresholds
    double velocityThreshold;           // votes per minute (default: 30)
    double deltaThresholdMs;            // avg inter-vote gap ms (default: 200)
    int minCoVotesForCollusion;         // minimum co-votes to flag (default: 5)
    double collusionThreshold;          // collusion score threshold (default: 0.7)
    double botLikelihoodThreshold;      // bot likelihood threshold (default: 0.7)
    int velocityWindowSeconds;          // sliding window size (default: 60)
    
    // Helper methods
    double calculateVotingVelocity(const std::string& userId);
    double calculateAvgInterVoteGap(const std::string& userId);
    bool detectBotBehavior(const std::string& userId);
    void updateBotDetection(const std::string& userId);
    void updateCollusionDetection();
    void generateThreatAlert(const std::string& alertType, double severity,
                            const std::vector<std::string>& users,
                            const std::string& description);
    
    double calculateAccountAgeScore(const std::string& userId);
    double calculateMajorityAgreementScore(const std::string& userId);
    
public:
    /**
     * Constructor
     * @param velThreshold Voting velocity threshold (votes/min)
     * @param deltaThreshold Average inter-vote gap threshold (ms)
     * @param windowSeconds Sliding window size for velocity tracking
     */
    AntiAbuseEngine(double velThreshold = 30.0,
                   double deltaThreshold = 200.0,
                   int windowSeconds = 60);
    
    /**
     * Record a vote event
     * @param userId User who voted
     * @param proposalId Proposal voted on
     * @param timestamp When the vote occurred
     * @param ipHash Hashed IP address (optional)
     * @param deviceHash Hashed device fingerprint (optional)
     */
    void recordVoteEvent(const std::string& userId,
                        const std::string& proposalId,
                        const std::chrono::system_clock::time_point& timestamp,
                        const std::string& ipHash = "",
                        const std::string& deviceHash = "");
    
    /**
     * Perform bot detection for a specific user
     * @param userId User to analyze
     * @return BotDetectionResult with analysis
     */
    BotDetectionResult detectBot(const std::string& userId);
    
    /**
     * Perform bot detection for all users
     * @return Vector of suspicious users
     */
    std::vector<BotDetectionResult> detectAllBots();
    
    /**
     * Perform collusion detection
     * Analyzes co-voting patterns to find suspicious groups
     * @return Vector of detected collusion groups
     */
    std::vector<CollusionDetectionResult> detectCollusion();
    
    /**
     * Calculate user credibility score
     * Combines multiple factors into overall trust score
     * @param userId User to score
     * @return UserCredibilityScore structure
     */
    UserCredibilityScore calculateUserCredibility(const std::string& userId);
    
    /**
     * Calculate credibility for all users
     */
    void calculateAllCredibilityScores();
    
    /**
     * Get user's trust score (0.0 to 1.0)
     * Returns default 0.5 if user not found
     * @param userId User identifier
     * @return Trust score
     */
    double getUserTrustScore(const std::string& userId);
    
    /**
     * Mark user as suspicious
     * @param userId User to flag
     * @param reason Reason for flagging
     */
    void markUserSuspicious(const std::string& userId, const std::string& reason);
    
    /**
     * Check if user is suspicious
     * @param userId User to check
     * @return True if flagged as suspicious
     */
    bool isUserSuspicious(const std::string& userId) const;
    
    /**
     * Get all threat alerts
     * @param unresolvedOnly If true, only return unresolved alerts
     * @return Vector of threat alerts
     */
    std::vector<ThreatAlert> getThreatAlerts(bool unresolvedOnly = false) const;
    
    /**
     * Resolve a threat alert
     * @param alertId Alert identifier
     */
    void resolveThreatAlert(const std::string& alertId);
    
    /**
     * Perform comprehensive security scan
     * Runs all detection algorithms and generates alerts
     * @return Summary string
     */
    std::string performSecurityScan();
    
    /**
     * Get security statistics
     * @return Formatted statistics string
     */
    std::string getSecurityStatistics() const;
    
    /**
     * Get vote count for user in time window
     * @param userId User identifier
     * @param windowSeconds Time window in seconds
     * @return Number of votes
     */
    int getVoteCountInWindow(const std::string& userId, int windowSeconds) const;
    
    /**
     * Get users sharing same IP
     * @param ipHash Hashed IP address
     * @return Vector of user IDs
     */
    std::vector<std::string> getUsersWithSameIP(const std::string& ipHash) const;
    
    /**
     * Get users sharing same device
     * @param deviceHash Hashed device fingerprint
     * @return Vector of user IDs
     */
    std::vector<std::string> getUsersWithSameDevice(const std::string& deviceHash) const;
    
    /**
     * Configure detection thresholds
     * @param velThreshold Velocity threshold (votes/min)
     * @param deltaThreshold Inter-vote gap threshold (ms)
     * @param collusionThresh Collusion score threshold
     * @param botThresh Bot likelihood threshold
     */
    void configureThresholds(double velThreshold,
                            double deltaThreshold,
                            double collusionThresh,
                            double botThresh);
    
    /**
     * Get current configuration
     * @return Configuration string
     */
    std::string getConfiguration() const;
    
    /**
     * Clear all data
     */
    void clearAll();
    
    /**
     * Clear user-specific data
     * @param userId User to clear
     */
    void clearUser(const std::string& userId);
};

#endif // ANTI_ABUSE_ENGINE_H
