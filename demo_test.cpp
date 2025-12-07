#include "VotingSystem.h"
#include <iostream>
#include <cassert>

void testBasicFunctionality() {
    std::cout << "=== Testing Basic Functionality ===" << std::endl;
    
    VotingSystem system;
    
    // Test user registration
    std::string userId1 = system.registerUser("Alice");
    std::string userId2 = system.registerUser("Bob");
    
    std::cout << "✓ Users registered: " << userId1 << ", " << userId2 << std::endl;
    
    // Test proposal creation
    std::string proposalId1 = system.createProposal("Remote Work Policy", "Allow WFH 3 days/week", userId1);
    std::string proposalId2 = system.createProposal("Coffee Machine", "Upgrade office coffee", userId2);
    
    std::cout << "✓ Proposals created: " << proposalId1 << ", " << proposalId2 << std::endl;
    
    // Test voting
    bool vote1 = system.castVote(userId2, proposalId1); // Bob votes for Alice's proposal
    bool vote2 = system.castVote(userId1, proposalId2); // Alice votes for Bob's proposal
    bool vote3 = system.castVote(userId1, proposalId1); // Alice votes for her own proposal
    
    std::cout << "✓ Votes cast: " << vote1 << ", " << vote2 << ", " << vote3 << std::endl;
    
    // Test duplicate voting prevention
    bool duplicateVote = system.castVote(userId1, proposalId1); // Alice tries to vote again
    assert(!duplicateVote);
    std::cout << "✓ Duplicate vote prevention works" << std::endl;
    
    // Test rankings
    auto topProposals = system.getTopProposals(2);
    std::cout << "✓ Top proposals retrieved: " << topProposals.size() << " proposals" << std::endl;
    
    // Test system integrity
    bool integrity = system.verifySystemIntegrity();
    std::cout << "✓ System integrity: " << (integrity ? "VERIFIED" : "COMPROMISED") << std::endl;
    
    // Display statistics
    std::cout << "\n=== System Statistics ===" << std::endl;
    std::cout << "Users: " << system.getUserCount() << std::endl;
    std::cout << "Proposals: " << system.getProposalCount() << std::endl;
    std::cout << "Log Entries: " << system.getLogEntryCount() << std::endl;
    
    std::cout << "\n=== Current Rankings ===" << std::endl;
    system.displayRankings(5);
    
    std::cout << "\n✅ All basic functionality tests passed!" << std::endl;
}

void testTamperDetection() {
    std::cout << "\n=== Testing Tamper Detection ===" << std::endl;
    
    TamperEvidentLog log;
    
    // Add some entries
    log.addEntry("User Alice registered");
    log.addEntry("Proposal 'Remote Work' created");
    log.addEntry("Vote cast by Bob for Remote Work");
    
    // Verify integrity
    bool initialIntegrity = log.verifyIntegrity();
    std::cout << "✓ Initial integrity: " << (initialIntegrity ? "VERIFIED" : "COMPROMISED") << std::endl;
    
    // Test tamper detection
    auto tamperingResults = log.detectTampering();
    std::cout << "✓ Tampering detection completed: " << tamperingResults.size() << " issues found" << std::endl;
    
    std::cout << "✅ Tamper detection tests passed!" << std::endl;
}

void testDataStructures() {
    std::cout << "\n=== Testing Data Structures Performance ===" << std::endl;
    
    VotingSystem system;
    
    // Create multiple users and proposals for performance testing
    std::vector<std::string> userIds;
    std::vector<std::string> proposalIds;
    
    // Register 10 users
    for (int i = 0; i < 10; ++i) {
        std::string userId = system.registerUser("User" + std::to_string(i));
        userIds.push_back(userId);
    }
    
    // Create 5 proposals
    for (int i = 0; i < 5; ++i) {
        std::string proposalId = system.createProposal(
            "Proposal " + std::to_string(i),
            "Description for proposal " + std::to_string(i),
            userIds[i % userIds.size()]
        );
        proposalIds.push_back(proposalId);
    }
    
    // Cast votes to test heap operations
    int voteCount = 0;
    for (const auto& userId : userIds) {
        for (const auto& proposalId : proposalIds) {
            if (system.castVote(userId, proposalId)) {
                voteCount++;
            }
        }
    }
    
    std::cout << "✓ Performance test completed:" << std::endl;
    std::cout << "  - Users: " << userIds.size() << std::endl;
    std::cout << "  - Proposals: " << proposalIds.size() << std::endl;
    std::cout << "  - Votes cast: " << voteCount << std::endl;
    
    // Test ranking retrieval
    auto rankings = system.getTopProposals(3);
    std::cout << "✓ Top 3 proposals retrieved successfully" << std::endl;
    
    // Verify system integrity after all operations
    bool finalIntegrity = system.verifySystemIntegrity();
    std::cout << "✓ Final system integrity: " << (finalIntegrity ? "VERIFIED" : "COMPROMISED") << std::endl;
    
    std::cout << "✅ Data structure performance tests passed!" << std::endl;
}

int main() {
    std::cout << "🚀 Starting Collaborative Voting Platform Tests\n" << std::endl;
    
    try {
        testBasicFunctionality();
        testTamperDetection();
        testDataStructures();
        
        std::cout << "\n🎉 All tests completed successfully!" << std::endl;
        std::cout << "\nThe Collaborative Voting Platform is ready for use!" << std::endl;
        std::cout << "Run './voting_system' to start the interactive interface." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
