#include "VotingSystem.h"
#include "IntelligenceEngine.h"
#include "AdvancedAnalytics.h"
#include <iostream>
#include <iomanip>

void printSeparator(const std::string& title = "") {
    std::cout << "\n" << std::string(70, '=') << "\n";
    if (!title.empty()) {
        std::cout << "  " << title << "\n";
        std::cout << std::string(70, '=') << "\n";
    }
}

void demonstrateNormalization() {
    printSeparator("NORMALIZATION DEMONSTRATION");
    
    std::vector<double> scores = {10.5, 25.3, 15.7, 30.2, 18.9, 22.1, 12.4};
    
    std::cout << "\nOriginal Scores: ";
    for (double s : scores) std::cout << std::fixed << std::setprecision(2) << s << " ";
    
    auto minMaxNorm = NormalizationUtils::minMaxNormalize(scores);
    std::cout << "\n\nMinMax Normalized (0-1 range): ";
    for (double s : minMaxNorm) std::cout << std::fixed << std::setprecision(3) << s << " ";
    
    auto zScore = NormalizationUtils::zScoreStandardize(scores);
    std::cout << "\n\nZ-Score Standardized (mean=0, std=1): ";
    for (double s : zScore) std::cout << std::fixed << std::setprecision(3) << s << " ";
    
    double mean = NormalizationUtils::calculateMean(scores);
    double stdDev = NormalizationUtils::calculateStdDev(scores, mean);
    std::cout << "\n\nStatistics:";
    std::cout << "\n  Mean: " << std::fixed << std::setprecision(3) << mean;
    std::cout << "\n  Std Dev: " << stdDev << "\n";
}

void demonstrateSimilarityMetrics() {
    printSeparator("SIMILARITY METRICS DEMONSTRATION");
    
    std::string text1 = "Implement renewable energy solutions for sustainable development";
    std::string text2 = "Green energy and sustainability initiatives for the environment";
    std::string text3 = "Budget allocation for healthcare infrastructure improvements";
    
    std::cout << "\nText 1: " << text1;
    std::cout << "\nText 2: " << text2;
    std::cout << "\nText 3: " << text3;
    
    // Jaccard Similarity
    auto tokens1 = NLPUtils::tokenize(text1);
    auto tokens2 = NLPUtils::tokenize(text2);
    auto tokens3 = NLPUtils::tokenize(text3);
    
    std::set<std::string> set1(tokens1.begin(), tokens1.end());
    std::set<std::string> set2(tokens2.begin(), tokens2.end());
    std::set<std::string> set3(tokens3.begin(), tokens3.end());
    
    double jaccard12 = SimilarityMetrics::jaccardSimilarity(set1, set2);
    double jaccard13 = SimilarityMetrics::jaccardSimilarity(set1, set3);
    
    std::cout << "\n\nJaccard Index:";
    std::cout << "\n  Text1 vs Text2: " << std::fixed << std::setprecision(3) << jaccard12;
    std::cout << "\n  Text1 vs Text3: " << jaccard13;
    
    // Combined Similarity (Jaccard + Cosine)
    double combined12 = SimilarityMetrics::combinedSimilarity(text1, text2);
    double combined13 = SimilarityMetrics::combinedSimilarity(text1, text3);
    
    std::cout << "\n\nCombined Similarity (Jaccard + Cosine):";
    std::cout << "\n  Text1 vs Text2: " << combined12;
    std::cout << "\n  Text1 vs Text3: " << combined13;
    std::cout << "\n\n✓ Higher similarity between Text1 and Text2 (both about environment)\n";
}

void demonstrateTopicAnalysis(VotingSystem& system) {
    printSeparator("TOPIC ANALYSIS & DECOMPOSITION");
    
    DecisionRankingEngine rankingEngine;
    TopicAnalyzer& analyzer = rankingEngine.getTopicAnalyzer();
    
    // Add sub-topics
    analyzer.decomposeTopicIntoSubTopics("TECH", {"AI", "Software", "Hardware", "Cybersecurity"});
    analyzer.decomposeTopicIntoSubTopics("ENV", {"Climate", "Renewable Energy", "Conservation"});
    
    std::cout << "\nTopic Hierarchy:\n";
    auto allTopics = analyzer.getAllTopics();
    for (const auto& topic : allTopics) {
        std::cout << "\n" << topic.name << " (" << topic.topicId << ")";
        if (!topic.subTopics.empty()) {
            std::cout << "\n  Sub-topics: ";
            for (const auto& sub : topic.subTopics) {
                std::cout << sub << " ";
            }
        }
        std::cout << "\n  Keywords (sorted for binary search): ";
        for (size_t i = 0; i < std::min(size_t(5), topic.keywords.size()); ++i) {
            std::cout << topic.keywords[i] << " ";
        }
        if (topic.keywords.size() > 5) std::cout << "...";
    }
    
    // Binary search demonstration
    std::cout << "\n\nBinary Search on Keywords:";
    std::cout << "\n  'technology' in TECH topic: " 
              << (analyzer.searchKeywordInTopic("TECH", "technology") ? "✓ Found" : "✗ Not found");
    std::cout << "\n  'climate' in ENV topic: " 
              << (analyzer.searchKeywordInTopic("ENV", "climate") ? "✓ Found" : "✗ Not found");
    std::cout << "\n  'random' in TECH topic: " 
              << (analyzer.searchKeywordInTopic("TECH", "random") ? "✓ Found" : "✗ Not found");
    std::cout << "\n";
}

void demonstrateLogisticRegression(VotingSystem& system) {
    printSeparator("LOGISTIC REGRESSION CLASSIFICATION");
    
    auto proposals = system.getAllProposals();
    if (proposals.empty()) {
        std::cout << "\nNo proposals available for classification.\n";
        return;
    }
    
    LogisticRegressionClassifier classifier(0.01, 500);
    
    // Create training labels (simplified)
    std::vector<std::string> labels;
    for (const auto& proposal : proposals) {
        if (proposal->getVoteCount() > 30) {
            labels.push_back("high_priority");
        } else if (proposal->getVoteCount() > 15) {
            labels.push_back("medium_priority");
        } else {
            labels.push_back("low_priority");
        }
    }
    
    std::cout << "\nTraining classifier with " << proposals.size() << " proposals...\n";
    classifier.train(proposals, labels);
    
    std::cout << "\nClassification Results:\n";
    for (size_t i = 0; i < std::min(size_t(3), proposals.size()); ++i) {
        auto result = classifier.classify(proposals[i]);
        std::cout << "\nProposal: " << proposals[i]->getTitle();
        std::cout << "\n  Votes: " << proposals[i]->getVoteCount();
        std::cout << "\n  Classification: " << result.label;
        std::cout << "\n  Confidence: " << std::fixed << std::setprecision(2) 
                  << (result.confidence * 100) << "%";
        
        auto probabilities = classifier.getClassProbabilities(proposals[i]);
        std::cout << "\n  All Probabilities:";
        for (const auto& pair : probabilities) {
            std::cout << "\n    " << pair.first << ": " 
                      << std::fixed << std::setprecision(2) << (pair.second * 100) << "%";
        }
    }
    std::cout << "\n";
}

void demonstrateTimeBasedFiltering(VotingSystem& system) {
    printSeparator("TIME-BASED FILTERING & PRIORITY");
    
    TimeBasedFilter timeFilter;
    auto proposals = system.getAllProposals();
    
    // Register proposals
    for (const auto& proposal : proposals) {
        timeFilter.registerProposal(proposal->getProposalId(), 
                                   proposal->getCreationTimestamp());
    }
    
    std::cout << "\nTime Filters:\n";
    
    TimeFilter recentFilter("recent", 24, 0.1);
    std::cout << "\n1. Recent Proposals (24 hours):";
    auto recent = timeFilter.getRecentProposals(24);
    std::cout << "\n   Found: " << recent.size() << " proposals";
    
    TimeFilter trendingFilter("trending", 6, 0.5);
    std::cout << "\n\n2. Trending Proposals (6 hours):";
    auto trending = timeFilter.getTrendingProposals(6);
    std::cout << "\n   Found: " << trending.size() << " proposals";
    
    std::cout << "\n\nTime Decay Scores (exponential decay):";
    for (size_t i = 0; i < std::min(size_t(3), proposals.size()); ++i) {
        double score = timeFilter.calculateTimeScore(proposals[i]->getProposalId(), recentFilter);
        std::cout << "\n  " << proposals[i]->getTitle().substr(0, 40) << "...";
        std::cout << "\n    Time Score: " << std::fixed << std::setprecision(3) << score;
    }
    std::cout << "\n";
}

void demonstrateDecisionRanking(VotingSystem& system) {
    printSeparator("DECISION RANKING BY TOPIC");
    
    auto proposals = system.getAllProposals();
    if (proposals.empty()) {
        std::cout << "\nNo proposals available for ranking.\n";
        return;
    }
    
    DecisionRankingEngine rankingEngine;
    rankingEngine.initialize(proposals);
    
    std::cout << "\nRanking decisions by TECH topic:\n";
    
    TimeFilter filter("recent", 168, 0.05);  // 1 week
    auto rankings = rankingEngine.rankDecisionsByTopic("TECH", proposals, filter);
    
    std::cout << "\n" << std::string(70, '-');
    std::cout << "\nRank | Title                          | Score | Percentile";
    std::cout << "\n" << std::string(70, '-');
    
    for (size_t i = 0; i < std::min(size_t(5), rankings.size()); ++i) {
        const auto& r = rankings[i];
        std::cout << "\n" << std::setw(4) << r.rank << " | ";
        std::cout << std::setw(30) << r.title.substr(0, 30) << " | ";
        std::cout << std::fixed << std::setprecision(3) << std::setw(5) << r.combinedScore << " | ";
        std::cout << std::fixed << std::setprecision(1) << std::setw(6) << r.percentile << "%";
        
        std::cout << "\n      Relevance: " << std::setprecision(2) << r.weightedRelevance;
        std::cout << " | Time: " << r.timeScore;
        std::cout << " | Priority: " << r.priorityScore;
        
        if (!r.matchedTopics.empty()) {
            std::cout << "\n      Topics: ";
            for (const auto& topic : r.matchedTopics) {
                std::cout << topic << " ";
            }
        }
    }
    
    std::cout << "\n" << std::string(70, '-');
    std::cout << rankingEngine.getRankingStatistics();
}

void demonstrateSimilarityMatrix(VotingSystem& system) {
    printSeparator("SIMILARITY MATRIX");
    
    auto proposals = system.getAllProposals();
    if (proposals.size() < 2) {
        std::cout << "\nNeed at least 2 proposals for similarity matrix.\n";
        return;
    }
    
    DecisionRankingEngine rankingEngine;
    rankingEngine.initialize(proposals);
    
    std::cout << "\nProposal Similarity Matrix (Jaccard + Cosine):\n\n";
    
    size_t displayCount = std::min(size_t(4), proposals.size());
    
    std::cout << std::setw(10) << " ";
    for (size_t i = 0; i < displayCount; ++i) {
        std::cout << std::setw(8) << ("P" + std::to_string(i+1));
    }
    std::cout << "\n";
    
    for (size_t i = 0; i < displayCount; ++i) {
        std::cout << std::setw(10) << ("P" + std::to_string(i+1));
        for (size_t j = 0; j < displayCount; ++j) {
            if (i == j) {
                std::cout << std::setw(8) << "1.000";
            } else {
                double sim = rankingEngine.getProposalSimilarity(
                    proposals[i]->getProposalId(), 
                    proposals[j]->getProposalId());
                std::cout << std::fixed << std::setprecision(3) << std::setw(8) << sim;
            }
        }
        std::cout << "\n";
    }
    
    std::cout << "\nProposal Titles:";
    for (size_t i = 0; i < displayCount; ++i) {
        std::cout << "\n  P" << (i+1) << ": " << proposals[i]->getTitle();
    }
    std::cout << "\n";
}

void demonstrateRankPercentile(VotingSystem& system) {
    printSeparator("RANK & PERCENTILE SYSTEM");
    
    auto proposals = system.getAllProposals();
    if (proposals.empty()) {
        std::cout << "\nNo proposals available.\n";
        return;
    }
    
    RankPercentileSystem rankSystem;
    
    std::vector<std::pair<std::string, double>> proposalScores;
    for (const auto& proposal : proposals) {
        double score = proposal->getVoteCount() * 1.5 + 
                      proposal->getVoters().size() * 2.0;
        proposalScores.emplace_back(proposal->getProposalId(), score);
    }
    
    rankSystem.updateRankings(proposalScores);
    
    std::cout << "\nProposal Rankings:\n";
    std::cout << std::string(70, '-');
    std::cout << "\nRank | Percentile | Title";
    std::cout << "\n" << std::string(70, '-');
    
    for (const auto& proposal : proposals) {
        int rank = rankSystem.getRank(proposal->getProposalId());
        double percentile = rankSystem.getPercentile(proposal->getProposalId());
        
        std::cout << "\n" << std::setw(4) << rank << " | ";
        std::cout << std::fixed << std::setprecision(1) << std::setw(9) << percentile << "% | ";
        std::cout << proposal->getTitle();
    }
    
    std::cout << "\n" << std::string(70, '-');
    std::cout << rankSystem.getStatistics();
    
    std::cout << "\n\nTop 25% Proposals:";
    auto top25 = rankSystem.getTopPercentProposals(75.0);
    std::cout << "\n  Count: " << top25.size();
    
    std::cout << "\n\nMiddle 50% Proposals (25th-75th percentile):";
    auto middle = rankSystem.getProposalsInPercentileRange(25.0, 75.0);
    std::cout << "\n  Count: " << middle.size() << "\n";
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     ADVANCED ANALYTICS & ML FEATURES DEMONSTRATION                 ║\n";
    std::cout << "║     Voting System with Enhanced NLP and Decision Ranking           ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";
    
    // Create voting system and populate with data
    VotingSystem system;
    
    std::cout << "\nInitializing system with sample data...\n";
    
    // Register users
    auto user1 = system.registerUser("Alice");
    auto user2 = system.registerUser("Bob");
    auto user3 = system.registerUser("Charlie");
    auto user4 = system.registerUser("Diana");
    auto user5 = system.registerUser("Eve");
    
    // Create proposals
    auto prop1 = system.createProposal(
        "AI-Powered Healthcare System",
        "Implement machine learning algorithms for early disease detection and diagnosis",
        user1);
    
    auto prop2 = system.createProposal(
        "Renewable Energy Infrastructure",
        "Build solar and wind energy facilities for sustainable power generation",
        user2);
    
    auto prop3 = system.createProposal(
        "Smart City Technology Platform",
        "Deploy IoT sensors and AI analytics for urban management and efficiency",
        user3);
    
    auto prop4 = system.createProposal(
        "Climate Action Initiative",
        "Comprehensive program for carbon reduction and environmental conservation",
        user4);
    
    auto prop5 = system.createProposal(
        "Digital Education Platform",
        "Online learning system with adaptive AI tutoring for students",
        user5);
    
    // Cast votes
    system.castVote(user1, prop2);
    system.castVote(user1, prop3);
    system.castVote(user2, prop1);
    system.castVote(user2, prop3);
    system.castVote(user2, prop4);
    system.castVote(user3, prop1);
    system.castVote(user3, prop2);
    system.castVote(user3, prop5);
    system.castVote(user4, prop1);
    system.castVote(user4, prop4);
    system.castVote(user5, prop3);
    system.castVote(user5, prop5);
    
    std::cout << "✓ Created " << system.getUserCount() << " users\n";
    std::cout << "✓ Created " << system.getProposalCount() << " proposals\n";
    std::cout << "✓ Cast 12 votes\n";
    
    // Run demonstrations
    demonstrateNormalization();
    demonstrateSimilarityMetrics();
    demonstrateTopicAnalysis(system);
    demonstrateLogisticRegression(system);
    demonstrateTimeBasedFiltering(system);
    demonstrateDecisionRanking(system);
    demonstrateSimilarityMatrix(system);
    demonstrateRankPercentile(system);
    
    printSeparator("DEMONSTRATION COMPLETE");
    std::cout << "\n✓ All advanced analytics features demonstrated successfully!\n";
    std::cout << "\nKey Features Implemented:\n";
    std::cout << "  ✓ MinMax Normalization & Z-Score Standardization\n";
    std::cout << "  ✓ Jaccard Index & Cosine Similarity\n";
    std::cout << "  ✓ Topic Decomposition with Binary Search\n";
    std::cout << "  ✓ Logistic Regression Classification\n";
    std::cout << "  ✓ Time-Based Filtering with Priority Queues\n";
    std::cout << "  ✓ Decision Ranking by Weighted Relevance\n";
    std::cout << "  ✓ Similarity Matrix Construction\n";
    std::cout << "  ✓ Rank & Percentile-Based Scoring\n\n";
    
    return 0;
}
