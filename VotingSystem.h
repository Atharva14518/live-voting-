#ifndef VOTING_SYSTEM_H
#define VOTING_SYSTEM_H

#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <memory>
#include <chrono>
#include <functional>
#include <iostream>
#include <iomanip>
#include <sstream>

// Forward declarations
class User;
class Proposal;
class Vote;
class TamperEvidentLog;

// Hash function for creating secure hashes
class HashUtils {
public:
    static std::string sha256(const std::string& data);
    static std::string getCurrentTimestamp();
    static std::string generateUserId();
    static std::string generateProposalId();
};

// Represents a user in the system
class User {
private:
    std::string userId;
    std::string username;
    std::string joinTimestamp;
    std::vector<std::string> votedProposals;

public:
    User(const std::string& username);
    
    // Getters
    const std::string& getUserId() const { return userId; }
    const std::string& getUsername() const { return username; }
    const std::string& getJoinTimestamp() const { return joinTimestamp; }
    const std::vector<std::string>& getVotedProposals() const { return votedProposals; }
    
    // Methods
    void addVotedProposal(const std::string& proposalId);
    bool hasVoted(const std::string& proposalId) const;
    std::string toString() const;
};

// Represents a proposal in the system
class Proposal {
private:
    std::string proposalId;
    std::string title;
    std::string description;
    std::string creatorId;
    std::string creationTimestamp;
    int voteCount;
    std::vector<std::string> voters;

public:
    Proposal(const std::string& title, const std::string& description, const std::string& creatorId);
    
    // Getters
    const std::string& getProposalId() const { return proposalId; }
    const std::string& getTitle() const { return title; }
    const std::string& getDescription() const { return description; }
    const std::string& getCreatorId() const { return creatorId; }
    const std::string& getCreationTimestamp() const { return creationTimestamp; }
    int getVoteCount() const { return voteCount; }
    const std::vector<std::string>& getVoters() const { return voters; }
    
    // Methods
    void addVote(const std::string& voterId);
    bool hasVoter(const std::string& voterId) const;
    std::string toString() const;
    
    // For priority queue comparison (max heap based on vote count)
    bool operator<(const Proposal& other) const {
        return voteCount < other.voteCount;
    }
};

// Represents a vote record
class Vote {
private:
    std::string voteId;
    std::string userId;
    std::string proposalId;
    std::string timestamp;
    std::string hash;

public:
    Vote(const std::string& userId, const std::string& proposalId);
    
    // Getters
    const std::string& getVoteId() const { return voteId; }
    const std::string& getUserId() const { return userId; }
    const std::string& getProposalId() const { return proposalId; }
    const std::string& getTimestamp() const { return timestamp; }
    const std::string& getHash() const { return hash; }
    
    // Methods
    std::string toString() const;
    std::string calculateHash(const std::string& previousHash = "") const;
};

// Tamper-evident log entry
class LogEntry {
private:
    std::string entryId;
    std::string data;
    std::string timestamp;
    std::string hash;
    std::string previousHash;

public:
    LogEntry(const std::string& data, const std::string& previousHash = "");
    
    // Getters
    const std::string& getEntryId() const { return entryId; }
    const std::string& getData() const { return data; }
    const std::string& getTimestamp() const { return timestamp; }
    const std::string& getHash() const { return hash; }
    const std::string& getPreviousHash() const { return previousHash; }
    
    // Methods
    std::string toString() const;
    bool verifyIntegrity(const std::string& expectedPreviousHash) const;
};

// Tamper-evident log using hash chaining
class TamperEvidentLog {
private:
    std::vector<LogEntry> entries;
    std::string lastHash;

public:
    TamperEvidentLog();
    
    // Methods
    void addEntry(const std::string& data);
    bool verifyIntegrity() const;
    void displayLog() const;
    size_t getSize() const { return entries.size(); }
    const LogEntry& getEntry(size_t index) const;
    
    // Tamper detection
    std::vector<size_t> detectTampering() const;
};

// Comparator for max heap of proposals
struct ProposalComparator {
    bool operator()(const std::shared_ptr<Proposal>& a, const std::shared_ptr<Proposal>& b) const {
        return a->getVoteCount() < b->getVoteCount();
    }
};

// Main voting system class
class VotingSystem {
private:
    std::unordered_map<std::string, std::shared_ptr<User>> users;
    std::unordered_map<std::string, std::shared_ptr<Proposal>> proposals;
    std::priority_queue<std::shared_ptr<Proposal>, 
                       std::vector<std::shared_ptr<Proposal>>, 
                       ProposalComparator> proposalRankings;
    TamperEvidentLog auditLog;
    class IntelligenceEngine* intelligenceEngine;
    
    // Helper methods
    void updateRankings();
    void logAction(const std::string& action);

public:
    VotingSystem();
    ~VotingSystem();
    
    // User management
    std::string registerUser(const std::string& username);
    std::shared_ptr<User> getUser(const std::string& userId);
    bool userExists(const std::string& userId) const;
    
    // Proposal management
    std::string createProposal(const std::string& title, const std::string& description, const std::string& creatorId);
    std::shared_ptr<Proposal> getProposal(const std::string& proposalId);
    bool proposalExists(const std::string& proposalId) const;
    
    // Voting
    bool castVote(const std::string& userId, const std::string& proposalId);
    
    // Rankings and display
    std::vector<std::shared_ptr<Proposal>> getTopProposals(int count = 10);
    void displayRankings(int count = 10);
    void displayUsers();
    void displayProposals();
    
    // Audit and integrity
    void displayAuditLog();
    bool verifySystemIntegrity();
    std::vector<std::string> detectTampering();
    
    // Statistics
    size_t getUserCount() const { return users.size(); }
    size_t getProposalCount() const { return proposals.size(); }
    size_t getLogEntryCount() const { return auditLog.getSize(); }
    
    // Intelligence features
    std::vector<std::string> getPersonalizedRecommendations(const std::string& userId, int maxResults = 5);
    std::string analyzeProposalSentiment(const std::string& proposalId);
    std::vector<std::string> performSecurityScan();
    std::string generateIntelligenceReport();
    std::vector<std::string> getPredictedTopProposals(int count = 5);
    
    // Helper method for intelligence engine
    std::vector<std::shared_ptr<Proposal>> getAllProposals() const;
};

#endif // VOTING_SYSTEM_H
