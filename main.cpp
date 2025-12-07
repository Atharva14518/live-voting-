#include "VotingSystem.h"
#include <iostream>
#include <string>
#include <limits>
#include <random>

class VotingInterface {
private:
    VotingSystem votingSystem;
    std::string currentUserId;
    
    void clearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }
    
    void pauseForInput() {
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }
    
    void displayHeader() {
        std::cout << "==========================================" << std::endl;
        std::cout << "    COLLABORATIVE VOTING PLATFORM" << std::endl;
        std::cout << "==========================================" << std::endl;
        if (!currentUserId.empty()) {
            auto user = votingSystem.getUser(currentUserId);
            if (user) {
                std::cout << "Logged in as: " << user->getUsername() << " (" << currentUserId << ")" << std::endl;
            }
        }
        std::cout << "==========================================" << std::endl;
    }
    
    void displayMainMenu() {
        std::cout << "\n=== MAIN MENU ===" << std::endl;
        std::cout << "1. Register/Login User" << std::endl;
        std::cout << "2. Create Proposal" << std::endl;
        std::cout << "3. Vote on Proposal" << std::endl;
        std::cout << "4. View Rankings" << std::endl;
        std::cout << "5. View All Proposals" << std::endl;
        std::cout << "6. View All Users" << std::endl;
        std::cout << "7. View Audit Log" << std::endl;
        std::cout << "8. System Integrity Check" << std::endl;
        std::cout << "9. System Statistics" << std::endl;
        std::cout << "10. Demo Mode (Auto-populate)" << std::endl;
        std::cout << "11. 🤖 AI Recommendations" << std::endl;
        std::cout << "12. 📊 Sentiment Analysis" << std::endl;
        std::cout << "13. 🔒 Security Scan" << std::endl;
        std::cout << "14. 🔮 Predictive Analytics" << std::endl;
        std::cout << "15. 📈 Intelligence Report" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << "\nEnter your choice: ";
    }
    
    void registerUser() {
        std::cout << "\n=== USER REGISTRATION ===" << std::endl;
        std::cout << "Enter username: ";
        std::string username;
        std::getline(std::cin, username);
        
        if (username.empty()) {
            std::cout << "Username cannot be empty!" << std::endl;
            return;
        }
        
        try {
            currentUserId = votingSystem.registerUser(username);
            std::cout << "User registered successfully!" << std::endl;
            std::cout << "Your User ID: " << currentUserId << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error registering user: " << e.what() << std::endl;
        }
    }
    
    void createProposal() {
        if (currentUserId.empty()) {
            std::cout << "Please register/login first!" << std::endl;
            return;
        }
        
        std::cout << "\n=== CREATE PROPOSAL ===" << std::endl;
        std::cout << "Enter proposal title: ";
        std::string title;
        std::getline(std::cin, title);
        
        if (title.empty()) {
            std::cout << "Title cannot be empty!" << std::endl;
            return;
        }
        
        std::cout << "Enter proposal description: ";
        std::string description;
        std::getline(std::cin, description);
        
        if (description.empty()) {
            std::cout << "Description cannot be empty!" << std::endl;
            return;
        }
        
        try {
            std::string proposalId = votingSystem.createProposal(title, description, currentUserId);
            std::cout << "Proposal created successfully!" << std::endl;
            std::cout << "Proposal ID: " << proposalId << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error creating proposal: " << e.what() << std::endl;
        }
    }
    
    void castVote() {
        if (currentUserId.empty()) {
            std::cout << "Please register/login first!" << std::endl;
            return;
        }
        
        std::cout << "\n=== CAST VOTE ===" << std::endl;
        
        // Show available proposals first
        votingSystem.displayProposals();
        
        std::cout << "\nEnter Proposal ID to vote for: ";
        std::string proposalId;
        std::getline(std::cin, proposalId);
        
        if (proposalId.empty()) {
            std::cout << "Proposal ID cannot be empty!" << std::endl;
            return;
        }
        
        bool success = votingSystem.castVote(currentUserId, proposalId);
        
        if (success) {
            std::cout << "Vote cast successfully!" << std::endl;
            std::cout << "Updated rankings:" << std::endl;
            votingSystem.displayRankings(5);
        } else {
            std::cout << "Failed to cast vote. Possible reasons:" << std::endl;
            std::cout << "- Proposal doesn't exist" << std::endl;
            std::cout << "- You've already voted for this proposal" << std::endl;
        }
    }
    
    void viewRankings() {
        std::cout << "\n=== VIEW RANKINGS ===" << std::endl;
        std::cout << "How many top proposals to show? (default 10): ";
        std::string input;
        std::getline(std::cin, input);
        
        int count = 10;
        if (!input.empty()) {
            try {
                count = std::stoi(input);
                if (count <= 0) count = 10;
            } catch (...) {
                count = 10;
            }
        }
        
        votingSystem.displayRankings(count);
    }
    
    void viewAuditLog() {
        std::cout << "\n=== AUDIT LOG ===" << std::endl;
        votingSystem.displayAuditLog();
    }
    
    void systemIntegrityCheck() {
        std::cout << "\n=== SYSTEM INTEGRITY CHECK ===" << std::endl;
        
        bool isIntact = votingSystem.verifySystemIntegrity();
        std::cout << "System Integrity: " << (isIntact ? "VERIFIED ✓" : "COMPROMISED ✗") << std::endl;
        
        auto tamperingResults = votingSystem.detectTampering();
        for (const auto& result : tamperingResults) {
            std::cout << result << std::endl;
        }
    }
    
    void showStatistics() {
        std::cout << "\n=== SYSTEM STATISTICS ===" << std::endl;
        std::cout << "Total Users: " << votingSystem.getUserCount() << std::endl;
        std::cout << "Total Proposals: " << votingSystem.getProposalCount() << std::endl;
        std::cout << "Total Log Entries: " << votingSystem.getLogEntryCount() << std::endl;
        
        // Calculate total votes
        size_t totalVotes = 0;
        auto topProposals = votingSystem.getTopProposals(1000); // Get all proposals
        for (const auto& proposal : topProposals) {
            totalVotes += proposal->getVoteCount();
        }
        std::cout << "Total Votes Cast: " << totalVotes << std::endl;
        
        std::cout << "System Integrity: " << (votingSystem.verifySystemIntegrity() ? "VERIFIED" : "COMPROMISED") << std::endl;
    }
    
    void demoMode() {
        std::cout << "\n=== DEMO MODE ===" << std::endl;
        std::cout << "Populating system with sample data..." << std::endl;
        
        // Register demo users
        std::vector<std::string> userIds;
        std::vector<std::string> usernames = {"Alice", "Bob", "Charlie", "Diana", "Eve", "Frank"};
        
        for (const auto& username : usernames) {
            std::string userId = votingSystem.registerUser(username);
            userIds.push_back(userId);
        }
        
        // Create demo proposals
        std::vector<std::string> proposalIds;
        std::vector<std::pair<std::string, std::string>> proposals = {
            {"Implement Remote Work Policy", "Allow employees to work from home 3 days per week"},
            {"Upgrade Office Coffee Machine", "Install a premium espresso machine in the break room"},
            {"Organize Team Building Event", "Plan a quarterly team outing to improve collaboration"},
            {"Flexible Working Hours", "Allow flexible start and end times within core hours"},
            {"Green Initiative Program", "Implement recycling and sustainability measures"}
        };
        
        for (size_t i = 0; i < proposals.size(); ++i) {
            std::string proposalId = votingSystem.createProposal(
                proposals[i].first, 
                proposals[i].second, 
                userIds[i % userIds.size()]
            );
            proposalIds.push_back(proposalId);
        }
        
        // Cast demo votes
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> voteDis(0, 1);
        
        for (const auto& userId : userIds) {
            for (const auto& proposalId : proposalIds) {
                if (voteDis(gen)) { // 50% chance to vote
                    votingSystem.castVote(userId, proposalId);
                }
            }
        }
        
        std::cout << "Demo data populated successfully!" << std::endl;
        std::cout << "- " << userIds.size() << " users registered" << std::endl;
        std::cout << "- " << proposalIds.size() << " proposals created" << std::endl;
        std::cout << "- Random votes cast" << std::endl;
        
        // Set current user to first demo user
        currentUserId = userIds[0];
        std::cout << "You are now logged in as: " << usernames[0] << std::endl;
    }
    
    void showAIRecommendations() {
        if (currentUserId.empty()) {
            std::cout << "Please register/login first!" << std::endl;
            return;
        }
        
        std::cout << "\n=== 🤖 AI RECOMMENDATIONS ===" << std::endl;
        auto recommendations = votingSystem.getPersonalizedRecommendations(currentUserId, 5);
        
        if (recommendations.empty()) {
            std::cout << "No recommendations available. Try voting on some proposals first!" << std::endl;
        } else {
            std::cout << "Based on your voting history, we recommend:" << std::endl;
            for (size_t i = 0; i < recommendations.size(); ++i) {
                std::cout << (i + 1) << ". " << recommendations[i] << std::endl;
            }
        }
    }
    
    void showSentimentAnalysis() {
        std::cout << "\n=== 📊 SENTIMENT ANALYSIS ===" << std::endl;
        std::cout << "Enter Proposal ID to analyze: ";
        std::string proposalId;
        std::getline(std::cin, proposalId);
        
        if (proposalId.empty()) {
            std::cout << "Proposal ID cannot be empty!" << std::endl;
            return;
        }
        
        std::string analysis = votingSystem.analyzeProposalSentiment(proposalId);
        std::cout << analysis << std::endl;
    }
    
    void performSecurityScan() {
        std::cout << "\n=== 🔒 SECURITY SCAN ===" << std::endl;
        std::cout << "Performing comprehensive security analysis..." << std::endl;
        
        auto results = votingSystem.performSecurityScan();
        for (const auto& result : results) {
            std::cout << result << std::endl;
        }
    }
    
    void showPredictiveAnalytics() {
        std::cout << "\n=== 🔮 PREDICTIVE ANALYTICS ===" << std::endl;
        auto predictions = votingSystem.getPredictedTopProposals(5);
        
        for (const auto& prediction : predictions) {
            std::cout << prediction << std::endl;
        }
    }
    
    void showIntelligenceReport() {
        std::cout << "\n=== 📈 INTELLIGENCE REPORT ===" << std::endl;
        std::string report = votingSystem.generateIntelligenceReport();
        std::cout << report << std::endl;
    }

public:
    void run() {
        std::cout << "Welcome to the Collaborative Voting Platform!" << std::endl;
        std::cout << "This system uses tamper-evident logging and real-time ranking." << std::endl;
        
        while (true) {
            clearScreen();
            displayHeader();
            displayMainMenu();
            
            std::string choice;
            std::getline(std::cin, choice);
            
            if (choice == "0") {
                std::cout << "Thank you for using the Collaborative Voting Platform!" << std::endl;
                break;
            }
            
            try {
                switch (std::stoi(choice)) {
                    case 1:
                        registerUser();
                        break;
                    case 2:
                        createProposal();
                        break;
                    case 3:
                        castVote();
                        break;
                    case 4:
                        viewRankings();
                        break;
                    case 5:
                        votingSystem.displayProposals();
                        break;
                    case 6:
                        votingSystem.displayUsers();
                        break;
                    case 7:
                        viewAuditLog();
                        break;
                    case 8:
                        systemIntegrityCheck();
                        break;
                    case 9:
                        showStatistics();
                        break;
                    case 10:
                        demoMode();
                        break;
                    case 11:
                        showAIRecommendations();
                        break;
                    case 12:
                        showSentimentAnalysis();
                        break;
                    case 13:
                        performSecurityScan();
                        break;
                    case 14:
                        showPredictiveAnalytics();
                        break;
                    case 15:
                        showIntelligenceReport();
                        break;
                    default:
                        std::cout << "Invalid choice! Please try again." << std::endl;
                }
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
            
            pauseForInput();
        }
    }
};

int main() {
    try {
        VotingInterface interface;
        interface.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
