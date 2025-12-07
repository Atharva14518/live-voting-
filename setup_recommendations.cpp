#include "VotingSystem.h"
#include <iostream>

int main() {
    std::cout << "=== SETTING UP AI RECOMMENDATIONS DEMO ===" << std::endl;
    
    VotingSystem system;
    
    // Step 1: Register users
    std::cout << "\n1. Registering users..." << std::endl;
    std::string alice = system.registerUser("Alice");
    std::string bob = system.registerUser("Bob");
    std::string charlie = system.registerUser("Charlie");
    
    std::cout << "✅ Registered: Alice (" << alice << ")" << std::endl;
    std::cout << "✅ Registered: Bob (" << bob << ")" << std::endl;
    std::cout << "✅ Registered: Charlie (" << charlie << ")" << std::endl;
    
    // Step 2: Create proposals
    std::cout << "\n2. Creating proposals..." << std::endl;
    std::string prop1 = system.createProposal(alice, "Coffee Machine Upgrade", 
        "Install a new espresso machine in the office kitchen for better coffee");
    std::string prop2 = system.createProposal(bob, "Remote Work Policy", 
        "Allow employees to work from home 2 days per week");
    std::string prop3 = system.createProposal(charlie, "Gym Membership Benefit", 
        "Company-sponsored gym memberships for all employees");
    std::string prop4 = system.createProposal(alice, "Flexible Hours Policy", 
        "Allow flexible working hours between 7 AM and 7 PM");
    
    std::cout << "✅ Created: Coffee Machine Upgrade (" << prop1 << ")" << std::endl;
    std::cout << "✅ Created: Remote Work Policy (" << prop2 << ")" << std::endl;
    std::cout << "✅ Created: Gym Membership Benefit (" << prop3 << ")" << std::endl;
    std::cout << "✅ Created: Flexible Hours Policy (" << prop4 << ")" << std::endl;
    
    // Step 3: Cast some votes to create voting history
    std::cout << "\n3. Creating voting history..." << std::endl;
    
    // Alice votes for remote work and gym
    system.vote(alice, prop2);
    system.vote(alice, prop3);
    std::cout << "✅ Alice voted for Remote Work and Gym Membership" << std::endl;
    
    // Bob votes for coffee machine and flexible hours
    system.vote(bob, prop1);
    system.vote(bob, prop4);
    std::cout << "✅ Bob voted for Coffee Machine and Flexible Hours" << std::endl;
    
    // Charlie votes for remote work and flexible hours
    system.vote(charlie, prop2);
    system.vote(charlie, prop4);
    std::cout << "✅ Charlie voted for Remote Work and Flexible Hours" << std::endl;
    
    // Step 4: Test AI recommendations
    std::cout << "\n4. Testing AI Recommendations..." << std::endl;
    
    auto aliceRecommendations = system.getIntelligenceEngine().getRecommendationsForUser(alice, 5);
    std::cout << "\n🤖 AI Recommendations for Alice:" << std::endl;
    if (aliceRecommendations.empty()) {
        std::cout << "❌ No recommendations available" << std::endl;
    } else {
        for (const auto& rec : aliceRecommendations) {
            std::cout << "  📋 Proposal: " << rec.proposalId << std::endl;
            std::cout << "  ⭐ Score: " << rec.score << std::endl;
            std::cout << "  💡 Reason: " << rec.reason << std::endl;
            std::cout << "  ---" << std::endl;
        }
    }
    
    auto bobRecommendations = system.getIntelligenceEngine().getRecommendationsForUser(bob, 5);
    std::cout << "\n🤖 AI Recommendations for Bob:" << std::endl;
    if (bobRecommendations.empty()) {
        std::cout << "❌ No recommendations available" << std::endl;
    } else {
        for (const auto& rec : bobRecommendations) {
            std::cout << "  📋 Proposal: " << rec.proposalId << std::endl;
            std::cout << "  ⭐ Score: " << rec.score << std::endl;
            std::cout << "  💡 Reason: " << rec.reason << std::endl;
            std::cout << "  ---" << std::endl;
        }
    }
    
    // Step 5: Show sentiment analysis
    std::cout << "\n5. Testing Sentiment Analysis..." << std::endl;
    auto sentiment1 = system.getIntelligenceEngine().analyzeProposalSentiment(prop1);
    std::cout << "📊 Coffee Machine sentiment: " << sentiment1.category 
              << " (score: " << sentiment1.overall << ")" << std::endl;
              
    auto sentiment2 = system.getIntelligenceEngine().analyzeProposalSentiment(prop2);
    std::cout << "📊 Remote Work sentiment: " << sentiment2.category 
              << " (score: " << sentiment2.overall << ")" << std::endl;
    
    // Step 6: Show intelligence report
    std::cout << "\n6. Intelligence Report:" << std::endl;
    std::cout << system.getIntelligenceEngine().generateInsightReport() << std::endl;
    
    std::cout << "\n=== DEMO COMPLETE ===" << std::endl;
    std::cout << "Now you can run the main program and see AI recommendations!" << std::endl;
    
    return 0;
}
