#include "VotingSystem.h"
#include "IntelligenceEngine.h"
#include "AdvancedAnalytics.h"
#include <iostream>
#include <iomanip>
#include <string>

void printSeparator(const std::string& title = "") {
    std::cout << "\n" << std::string(70, '=') << "\n";
    if (!title.empty()) {
        std::cout << "  " << title << "\n";
        std::cout << std::string(70, '=') << "\n";
    }
}

int main() {
    std::cout << "\n╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     CUSTOM PROPOSAL ANALYSIS                                       ║\n";
    std::cout << "║     Analyze Your Own Proposals with Advanced Analytics             ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";
    
    VotingSystem system;
    
    // Step 1: Register users
    std::cout << "\n=== STEP 1: Register Users ===\n";
    std::cout << "How many users do you want to register? ";
    int numUsers;
    std::cin >> numUsers;
    std::cin.ignore(); // Clear newline
    
    std::vector<std::string> userIds;
    for (int i = 0; i < numUsers; ++i) {
        std::string username;
        std::cout << "Enter username " << (i + 1) << ": ";
        std::getline(std::cin, username);
        
        std::string userId = system.registerUser(username);
        userIds.push_back(userId);
        std::cout << "  ✓ Registered: " << username << " (ID: " << userId << ")\n";
    }
    
    // Step 2: Create proposals
    std::cout << "\n=== STEP 2: Create Proposals ===\n";
    std::cout << "How many proposals do you want to create? ";
    int numProposals;
    std::cin >> numProposals;
    std::cin.ignore();
    
    std::vector<std::string> proposalIds;
    for (int i = 0; i < numProposals; ++i) {
        std::string title, description;
        int creatorIndex;
        
        std::cout << "\n--- Proposal " << (i + 1) << " ---\n";
        std::cout << "Title: ";
        std::getline(std::cin, title);
        
        std::cout << "Description: ";
        std::getline(std::cin, description);
        
        std::cout << "Creator (enter user number 1-" << numUsers << "): ";
        std::cin >> creatorIndex;
        std::cin.ignore();
        
        if (creatorIndex < 1 || creatorIndex > numUsers) {
            std::cout << "  ✗ Invalid user number, using user 1\n";
            creatorIndex = 1;
        }
        
        std::string proposalId = system.createProposal(title, description, 
                                                       userIds[creatorIndex - 1]);
        proposalIds.push_back(proposalId);
        std::cout << "  ✓ Created proposal: " << title << "\n";
    }
    
    // Step 3: Cast votes (optional)
    std::cout << "\n=== STEP 3: Cast Votes (Optional) ===\n";
    std::cout << "Do you want to cast votes? (y/n): ";
    char castVotes;
    std::cin >> castVotes;
    std::cin.ignore();
    
    if (castVotes == 'y' || castVotes == 'Y') {
        std::cout << "How many votes to cast? ";
        int numVotes;
        std::cin >> numVotes;
        std::cin.ignore();
        
        for (int i = 0; i < numVotes; ++i) {
            int userIdx, proposalIdx;
            std::cout << "\nVote " << (i + 1) << ":\n";
            std::cout << "  User (1-" << numUsers << "): ";
            std::cin >> userIdx;
            std::cout << "  Proposal (1-" << numProposals << "): ";
            std::cin >> proposalIdx;
            std::cin.ignore();
            
            if (userIdx >= 1 && userIdx <= numUsers && 
                proposalIdx >= 1 && proposalIdx <= numProposals) {
                
                bool success = system.castVote(userIds[userIdx - 1], 
                                              proposalIds[proposalIdx - 1]);
                if (success) {
                    std::cout << "  ✓ Vote cast successfully\n";
                } else {
                    std::cout << "  ✗ Vote failed (already voted?)\n";
                }
            } else {
                std::cout << "  ✗ Invalid user or proposal number\n";
            }
        }
    }
    
    // Step 4: Run Advanced Analytics
    std::cout << "\n\n";
    printSeparator("ADVANCED ANALYTICS RESULTS");
    
    auto proposals = system.getAllProposals();
    
    // 1. Similarity Analysis
    printSeparator("1. SIMILARITY ANALYSIS");
    if (proposals.size() >= 2) {
        std::cout << "\nPairwise Similarity (Jaccard + Cosine):\n\n";
        
        for (size_t i = 0; i < proposals.size(); ++i) {
            for (size_t j = i + 1; j < proposals.size(); ++j) {
                std::string text1 = proposals[i]->getTitle() + " " + 
                                   proposals[i]->getDescription();
                std::string text2 = proposals[j]->getTitle() + " " + 
                                   proposals[j]->getDescription();
                
                double similarity = SimilarityMetrics::combinedSimilarity(text1, text2);
                
                std::cout << "Proposal " << (i + 1) << " ↔ Proposal " << (j + 1) << ": "
                          << std::fixed << std::setprecision(3) << similarity;
                
                if (similarity > 0.3) {
                    std::cout << " (High similarity)";
                } else if (similarity > 0.15) {
                    std::cout << " (Moderate similarity)";
                } else {
                    std::cout << " (Low similarity)";
                }
                std::cout << "\n";
            }
        }
    } else {
        std::cout << "\nNeed at least 2 proposals for similarity analysis.\n";
    }
    
    // 2. Topic Analysis
    printSeparator("2. TOPIC ANALYSIS");
    DecisionRankingEngine rankingEngine;
    rankingEngine.initialize(proposals);
    
    TopicAnalyzer& analyzer = rankingEngine.getTopicAnalyzer();
    
    std::cout << "\nProposal Topics:\n";
    for (size_t i = 0; i < proposals.size(); ++i) {
        auto topics = analyzer.getProposalTopics(proposals[i]->getProposalId());
        
        std::cout << "\nProposal " << (i + 1) << ": " << proposals[i]->getTitle() << "\n";
        std::cout << "  Topics: ";
        if (topics.empty()) {
            std::cout << "None detected";
        } else {
            for (const auto& topicId : topics) {
                Topic topic = analyzer.getTopic(topicId);
                std::cout << topic.name << " ";
            }
        }
        std::cout << "\n";
    }
    
    // 3. Classification
    printSeparator("3. LOGISTIC REGRESSION CLASSIFICATION");
    LogisticRegressionClassifier classifier(0.01, 500);
    
    // Create labels based on vote counts
    std::vector<std::string> labels;
    for (const auto& proposal : proposals) {
        if (proposal->getVoteCount() > 5) {
            labels.push_back("high_priority");
        } else if (proposal->getVoteCount() > 2) {
            labels.push_back("medium_priority");
        } else {
            labels.push_back("low_priority");
        }
    }
    
    classifier.train(proposals, labels);
    
    std::cout << "\nClassification Results:\n";
    for (size_t i = 0; i < proposals.size(); ++i) {
        auto result = classifier.classify(proposals[i]);
        
        std::cout << "\nProposal " << (i + 1) << ": " << proposals[i]->getTitle() << "\n";
        std::cout << "  Votes: " << proposals[i]->getVoteCount() << "\n";
        std::cout << "  Classification: " << result.label << "\n";
        std::cout << "  Confidence: " << std::fixed << std::setprecision(1) 
                  << (result.confidence * 100) << "%\n";
    }
    
    // 4. Decision Ranking by Topic
    printSeparator("4. DECISION RANKING BY TOPIC");
    
    std::cout << "\nAvailable Topics:\n";
    auto allTopics = analyzer.getAllTopics();
    for (size_t i = 0; i < allTopics.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << allTopics[i].name 
                  << " (" << allTopics[i].topicId << ")\n";
    }
    
    std::cout << "\nSelect topic for ranking (1-" << allTopics.size() << "): ";
    int topicChoice;
    std::cin >> topicChoice;
    
    if (topicChoice >= 1 && topicChoice <= static_cast<int>(allTopics.size())) {
        std::string selectedTopicId = allTopics[topicChoice - 1].topicId;
        
        TimeFilter filter("recent", 168, 0.05);  // 1 week
        auto rankings = rankingEngine.rankDecisionsByTopic(selectedTopicId, proposals, filter);
        
        std::cout << "\nRankings for " << allTopics[topicChoice - 1].name << " topic:\n";
        std::cout << std::string(70, '-') << "\n";
        std::cout << "Rank | Title                          | Score | Percentile\n";
        std::cout << std::string(70, '-') << "\n";
        
        for (const auto& r : rankings) {
            std::cout << std::setw(4) << r.rank << " | ";
            std::cout << std::setw(30) << r.title.substr(0, 30) << " | ";
            std::cout << std::fixed << std::setprecision(3) << std::setw(5) << r.combinedScore << " | ";
            std::cout << std::fixed << std::setprecision(1) << std::setw(6) << r.percentile << "%\n";
        }
        std::cout << std::string(70, '-') << "\n";
    }
    
    // 5. Rank & Percentile System
    printSeparator("5. RANK & PERCENTILE SYSTEM");
    
    RankPercentileSystem rankSystem;
    std::vector<std::pair<std::string, double>> proposalScores;
    
    for (const auto& proposal : proposals) {
        double score = proposal->getVoteCount() * 2.0 + 
                      proposal->getVoters().size() * 1.5;
        proposalScores.emplace_back(proposal->getProposalId(), score);
    }
    
    rankSystem.updateRankings(proposalScores);
    
    std::cout << "\nOverall Rankings:\n";
    std::cout << std::string(70, '-') << "\n";
    std::cout << "Rank | Percentile | Votes | Title\n";
    std::cout << std::string(70, '-') << "\n";
    
    for (const auto& proposal : proposals) {
        int rank = rankSystem.getRank(proposal->getProposalId());
        double percentile = rankSystem.getPercentile(proposal->getProposalId());
        
        std::cout << std::setw(4) << rank << " | ";
        std::cout << std::fixed << std::setprecision(1) << std::setw(9) << percentile << "% | ";
        std::cout << std::setw(5) << proposal->getVoteCount() << " | ";
        std::cout << proposal->getTitle() << "\n";
    }
    std::cout << std::string(70, '-') << "\n";
    
    std::cout << rankSystem.getStatistics();
    
    // 6. Normalization Example
    printSeparator("6. SCORE NORMALIZATION");
    
    std::vector<double> scores;
    for (const auto& proposal : proposals) {
        scores.push_back(static_cast<double>(proposal->getVoteCount()));
    }
    
    if (!scores.empty()) {
        std::cout << "\nOriginal Vote Counts: ";
        for (double s : scores) {
            std::cout << std::fixed << std::setprecision(0) << s << " ";
        }
        
        auto normalized = NormalizationUtils::minMaxNormalize(scores);
        std::cout << "\n\nMinMax Normalized [0-1]: ";
        for (double s : normalized) {
            std::cout << std::fixed << std::setprecision(3) << s << " ";
        }
        
        auto standardized = NormalizationUtils::zScoreStandardize(scores);
        std::cout << "\n\nZ-Score Standardized: ";
        for (double s : standardized) {
            std::cout << std::fixed << std::setprecision(3) << s << " ";
        }
        std::cout << "\n";
    }
    
    printSeparator("ANALYSIS COMPLETE");
    std::cout << "\n✓ All advanced analytics completed successfully!\n";
    std::cout << "\nSummary:\n";
    std::cout << "  • " << system.getUserCount() << " users registered\n";
    std::cout << "  • " << system.getProposalCount() << " proposals created\n";
    std::cout << "  • Similarity analysis performed\n";
    std::cout << "  • Topic analysis completed\n";
    std::cout << "  • Classification executed\n";
    std::cout << "  • Rankings calculated\n";
    std::cout << "  • Percentiles computed\n\n";
    
    return 0;
}
