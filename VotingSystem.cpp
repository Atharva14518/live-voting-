#include "VotingSystem.h"
#include "IntelligenceEngine.h"
#include <random>
#include <algorithm>
#include <iomanip>

// Simple hash function (in production, use a proper cryptographic hash)
std::string HashUtils::sha256(const std::string& data) {
    std::hash<std::string> hasher;
    size_t hashValue = hasher(data);
    
    std::stringstream ss;
    ss << std::hex << hashValue;
    return ss.str();
}

std::string HashUtils::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string HashUtils::generateUserId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);
    
    return "USER_" + std::to_string(dis(gen));
}

std::string HashUtils::generateProposalId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);
    
    return "PROP_" + std::to_string(dis(gen));
}

// User implementation
User::User(const std::string& username) 
    : userId(HashUtils::generateUserId()), username(username), joinTimestamp(HashUtils::getCurrentTimestamp()) {
}

void User::addVotedProposal(const std::string& proposalId) {
    if (!hasVoted(proposalId)) {
        votedProposals.push_back(proposalId);
    }
}

bool User::hasVoted(const std::string& proposalId) const {
    return std::find(votedProposals.begin(), votedProposals.end(), proposalId) != votedProposals.end();
}

std::string User::toString() const {
    std::stringstream ss;
    ss << "User ID: " << userId << "\n";
    ss << "Username: " << username << "\n";
    ss << "Joined: " << joinTimestamp << "\n";
    ss << "Votes Cast: " << votedProposals.size() << "\n";
    return ss.str();
}

// Proposal implementation
Proposal::Proposal(const std::string& title, const std::string& description, const std::string& creatorId)
    : proposalId(HashUtils::generateProposalId()), title(title), description(description), 
      creatorId(creatorId), creationTimestamp(HashUtils::getCurrentTimestamp()), voteCount(0) {
}

void Proposal::addVote(const std::string& voterId) {
    if (!hasVoter(voterId)) {
        voters.push_back(voterId);
        voteCount++;
    }
}

bool Proposal::hasVoter(const std::string& voterId) const {
    return std::find(voters.begin(), voters.end(), voterId) != voters.end();
}

std::string Proposal::toString() const {
    std::stringstream ss;
    ss << "Proposal ID: " << proposalId << "\n";
    ss << "Title: " << title << "\n";
    ss << "Description: " << description << "\n";
    ss << "Creator: " << creatorId << "\n";
    ss << "Created: " << creationTimestamp << "\n";
    ss << "Vote Count: " << voteCount << "\n";
    return ss.str();
}

// Vote implementation
Vote::Vote(const std::string& userId, const std::string& proposalId)
    : userId(userId), proposalId(proposalId), timestamp(HashUtils::getCurrentTimestamp()) {
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);
    
    voteId = "VOTE_" + std::to_string(dis(gen));
    hash = calculateHash();
}

std::string Vote::toString() const {
    std::stringstream ss;
    ss << "Vote ID: " << voteId << "\n";
    ss << "User ID: " << userId << "\n";
    ss << "Proposal ID: " << proposalId << "\n";
    ss << "Timestamp: " << timestamp << "\n";
    ss << "Hash: " << hash << "\n";
    return ss.str();
}

std::string Vote::calculateHash(const std::string& previousHash) const {
    std::string data = voteId + userId + proposalId + timestamp + previousHash;
    return HashUtils::sha256(data);
}

// LogEntry implementation
LogEntry::LogEntry(const std::string& data, const std::string& previousHash)
    : data(data), timestamp(HashUtils::getCurrentTimestamp()), previousHash(previousHash) {
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);
    
    entryId = "LOG_" + std::to_string(dis(gen));
    
    std::string hashInput = entryId + data + timestamp + previousHash;
    hash = HashUtils::sha256(hashInput);
}

std::string LogEntry::toString() const {
    std::stringstream ss;
    ss << "Entry ID: " << entryId << "\n";
    ss << "Timestamp: " << timestamp << "\n";
    ss << "Data: " << data << "\n";
    ss << "Hash: " << hash << "\n";
    ss << "Previous Hash: " << previousHash << "\n";
    return ss.str();
}

bool LogEntry::verifyIntegrity(const std::string& expectedPreviousHash) const {
    if (previousHash != expectedPreviousHash) {
        return false;
    }
    
    std::string hashInput = entryId + data + timestamp + previousHash;
    std::string calculatedHash = HashUtils::sha256(hashInput);
    
    return hash == calculatedHash;
}

// TamperEvidentLog implementation
TamperEvidentLog::TamperEvidentLog() : lastHash("GENESIS") {
}

void TamperEvidentLog::addEntry(const std::string& data) {
    LogEntry entry(data, lastHash);
    entries.push_back(entry);
    lastHash = entry.getHash();
}

bool TamperEvidentLog::verifyIntegrity() const {
    if (entries.empty()) {
        return true;
    }
    
    std::string expectedPreviousHash = "GENESIS";
    
    for (const auto& entry : entries) {
        if (!entry.verifyIntegrity(expectedPreviousHash)) {
            return false;
        }
        expectedPreviousHash = entry.getHash();
    }
    
    return true;
}

void TamperEvidentLog::displayLog() const {
    std::cout << "\n=== AUDIT LOG ===" << std::endl;
    std::cout << "Total entries: " << entries.size() << std::endl;
    std::cout << "Integrity status: " << (verifyIntegrity() ? "VERIFIED" : "COMPROMISED") << std::endl;
    
    for (size_t i = 0; i < entries.size(); ++i) {
        std::cout << "\n--- Entry " << (i + 1) << " ---" << std::endl;
        std::cout << entries[i].toString() << std::endl;
    }
}

const LogEntry& TamperEvidentLog::getEntry(size_t index) const {
    if (index >= entries.size()) {
        throw std::out_of_range("Log entry index out of range");
    }
    return entries[index];
}

std::vector<size_t> TamperEvidentLog::detectTampering() const {
    std::vector<size_t> tamperedEntries;
    
    if (entries.empty()) {
        return tamperedEntries;
    }
    
    std::string expectedPreviousHash = "GENESIS";
    
    for (size_t i = 0; i < entries.size(); ++i) {
        if (!entries[i].verifyIntegrity(expectedPreviousHash)) {
            tamperedEntries.push_back(i);
        }
        expectedPreviousHash = entries[i].getHash();
    }
    
    return tamperedEntries;
}

// VotingSystem implementation
VotingSystem::VotingSystem() : intelligenceEngine(nullptr) {
    intelligenceEngine = new IntelligenceEngine(this);
    logAction("System initialized with Intelligence Engine");
}

VotingSystem::~VotingSystem() {
    delete intelligenceEngine;
}

void VotingSystem::updateRankings() {
    // Clear current rankings
    while (!proposalRankings.empty()) {
        proposalRankings.pop();
    }
    
    // Rebuild rankings with current vote counts
    for (const auto& pair : proposals) {
        proposalRankings.push(pair.second);
    }
}

void VotingSystem::logAction(const std::string& action) {
    auditLog.addEntry(action);
}

std::string VotingSystem::registerUser(const std::string& username) {
    auto user = std::make_shared<User>(username);
    std::string userId = user->getUserId();
    
    users[userId] = user;
    logAction("User registered: " + username + " (ID: " + userId + ")");
    
    return userId;
}

std::shared_ptr<User> VotingSystem::getUser(const std::string& userId) {
    auto it = users.find(userId);
    return (it != users.end()) ? it->second : nullptr;
}

bool VotingSystem::userExists(const std::string& userId) const {
    return users.find(userId) != users.end();
}

std::string VotingSystem::createProposal(const std::string& title, const std::string& description, const std::string& creatorId) {
    if (!userExists(creatorId)) {
        throw std::invalid_argument("Creator user does not exist");
    }
    
    auto proposal = std::make_shared<Proposal>(title, description, creatorId);
    std::string proposalId = proposal->getProposalId();
    
    proposals[proposalId] = proposal;
    proposalRankings.push(proposal);
    
    logAction("Proposal created: " + title + " (ID: " + proposalId + ") by " + creatorId);
    
    return proposalId;
}

std::shared_ptr<Proposal> VotingSystem::getProposal(const std::string& proposalId) {
    auto it = proposals.find(proposalId);
    return (it != proposals.end()) ? it->second : nullptr;
}

bool VotingSystem::proposalExists(const std::string& proposalId) const {
    return proposals.find(proposalId) != proposals.end();
}

bool VotingSystem::castVote(const std::string& userId, const std::string& proposalId) {
    auto user = getUser(userId);
    auto proposal = getProposal(proposalId);
    
    if (!user || !proposal) {
        return false;
    }
    
    if (user->hasVoted(proposalId)) {
        return false; // User has already voted for this proposal
    }
    
    // Cast the vote
    user->addVotedProposal(proposalId);
    proposal->addVote(userId);
    
    // Update rankings
    updateRankings();
    
    // Log the vote
    Vote vote(userId, proposalId);
    logAction("Vote cast: " + vote.toString());
    
    // Let intelligence engine learn from this vote
    if (intelligenceEngine) {
        intelligenceEngine->learnFromVote(userId, proposalId);
    }
    
    return true;
}

std::vector<std::shared_ptr<Proposal>> VotingSystem::getTopProposals(int count) {
    std::vector<std::shared_ptr<Proposal>> topProposals;
    
    // Create a copy of the priority queue to avoid modifying the original
    auto tempQueue = proposalRankings;
    
    for (int i = 0; i < count && !tempQueue.empty(); ++i) {
        topProposals.push_back(tempQueue.top());
        tempQueue.pop();
    }
    
    return topProposals;
}

void VotingSystem::displayRankings(int count) {
    std::cout << "\n=== TOP PROPOSALS RANKING ===" << std::endl;
    
    auto topProposals = getTopProposals(count);
    
    if (topProposals.empty()) {
        std::cout << "No proposals available." << std::endl;
        return;
    }
    
    for (size_t i = 0; i < topProposals.size(); ++i) {
        std::cout << "\n--- Rank " << (i + 1) << " ---" << std::endl;
        std::cout << "Title: " << topProposals[i]->getTitle() << std::endl;
        std::cout << "Votes: " << topProposals[i]->getVoteCount() << std::endl;
        std::cout << "ID: " << topProposals[i]->getProposalId() << std::endl;
    }
}

void VotingSystem::displayUsers() {
    std::cout << "\n=== REGISTERED USERS ===" << std::endl;
    std::cout << "Total users: " << users.size() << std::endl;
    
    for (const auto& pair : users) {
        std::cout << "\n" << pair.second->toString() << std::endl;
    }
}

void VotingSystem::displayProposals() {
    std::cout << "\n=== ALL PROPOSALS ===" << std::endl;
    std::cout << "Total proposals: " << proposals.size() << std::endl;
    
    for (const auto& pair : proposals) {
        std::cout << "\n" << pair.second->toString() << std::endl;
    }
}

void VotingSystem::displayAuditLog() {
    auditLog.displayLog();
}

bool VotingSystem::verifySystemIntegrity() {
    return auditLog.verifyIntegrity();
}

std::vector<std::string> VotingSystem::detectTampering() {
    std::vector<std::string> results;
    auto tamperedEntries = auditLog.detectTampering();
    
    if (tamperedEntries.empty()) {
        results.push_back("No tampering detected. System integrity verified.");
    } else {
        results.push_back("ALERT: Tampering detected in " + std::to_string(tamperedEntries.size()) + " log entries:");
        for (size_t index : tamperedEntries) {
            results.push_back("  - Entry " + std::to_string(index + 1) + " has been tampered with");
        }
    }
    
    return results;
}

// Intelligence features implementation
std::vector<std::string> VotingSystem::getPersonalizedRecommendations(const std::string& userId, int maxResults) {
    std::vector<std::string> recommendations;
    
    if (!intelligenceEngine) {
        return recommendations;
    }
    
    auto results = intelligenceEngine->getRecommendationsForUser(userId, maxResults);
    for (const auto& result : results) {
        recommendations.push_back("Proposal " + result.proposalId + " (Score: " + 
                                std::to_string(result.score) + ") - " + result.reason);
    }
    
    return recommendations;
}

std::string VotingSystem::analyzeProposalSentiment(const std::string& proposalId) {
    if (!intelligenceEngine) {
        return "Intelligence engine not available";
    }
    
    auto sentiment = intelligenceEngine->analyzeProposalSentiment(proposalId);
    
    std::stringstream ss;
    ss << "Sentiment Analysis for Proposal " << proposalId << ":\n";
    ss << "Overall Score: " << std::fixed << std::setprecision(2) << sentiment.overall << "\n";
    ss << "Category: " << sentiment.category << "\n";
    ss << "Positivity: " << std::fixed << std::setprecision(2) << sentiment.positivity << "\n";
    ss << "Negativity: " << std::fixed << std::setprecision(2) << sentiment.negativity << "\n";
    ss << "Neutrality: " << std::fixed << std::setprecision(2) << sentiment.neutrality;
    
    return ss.str();
}

std::vector<std::string> VotingSystem::performSecurityScan() {
    std::vector<std::string> results;
    
    if (!intelligenceEngine) {
        results.push_back("Intelligence engine not available");
        return results;
    }
    
    auto anomalies = intelligenceEngine->performSecurityScan();
    
    if (anomalies.empty()) {
        results.push_back("✓ No security anomalies detected");
        results.push_back("✓ All user behavior appears normal");
        results.push_back("✓ No bot activity detected");
        results.push_back("✓ No collusion patterns found");
    } else {
        results.push_back("⚠ SECURITY ALERTS DETECTED:");
        for (const auto& anomaly : anomalies) {
            std::string severity;
            if (anomaly.severity > 0.8) severity = "HIGH";
            else if (anomaly.severity > 0.5) severity = "MEDIUM";
            else severity = "LOW";
            
            results.push_back("  [" + severity + "] " + anomaly.type + ": " + anomaly.description);
        }
    }
    
    return results;
}

std::string VotingSystem::generateIntelligenceReport() {
    if (!intelligenceEngine) {
        return "Intelligence engine not available";
    }
    
    return intelligenceEngine->generateInsightReport();
}

std::vector<std::string> VotingSystem::getPredictedTopProposals(int count) {
    std::vector<std::string> predictions;
    
    if (!intelligenceEngine) {
        predictions.push_back("Intelligence engine not available");
        return predictions;
    }
    
    auto rankings = intelligenceEngine->getPredictedRankings();
    
    predictions.push_back("=== PREDICTED TOP PROPOSALS ===");
    for (int i = 0; i < std::min(count, static_cast<int>(rankings.size())); ++i) {
        auto proposal = getProposal(rankings[i].first);
        if (proposal) {
            predictions.push_back(std::to_string(i + 1) + ". " + proposal->getTitle() + 
                                " (Predicted votes: " + std::to_string(static_cast<int>(rankings[i].second)) + ")");
        }
    }
    
    return predictions;
}

std::vector<std::shared_ptr<Proposal>> VotingSystem::getAllProposals() const {
    std::vector<std::shared_ptr<Proposal>> allProposals;
    for (const auto& pair : proposals) {
        allProposals.push_back(pair.second);
    }
    return allProposals;
}
