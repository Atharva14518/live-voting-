#include "VotingSystem.h"
#include "ConsistencyScorer.h"
#include "AntiAbuseEngine.h"
#include "EnsembleModels.h"
#include "StreamProcessor.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

using namespace std;

void printHeader(const string& title) {
    cout << "\n" << string(70, '=') << "\n";
    cout << "  " << title << "\n";
    cout << string(70, '=') << "\n\n";
}

void printSubHeader(const string& title) {
    cout << "\n--- " << title << " ---\n\n";
}

void demonstrateConsistencyScoring() {
    printHeader("CONSISTENCY & STABILITY SCORING");
    
    ConsistencyScorer scorer(50, true);
    
    // Display weight configuration
    cout << ConsistencyScorer::getWeightConfiguration() << "\n";
    
    printSubHeader("Simulating User Proposal History");
    
    // User 1: Consistent, stable proposals (high consistency)
    string user1 = "USER_CONSISTENT";
    cout << "User 1 (" << user1 << "): Consistently submits high-quality proposals\n";
    for (int i = 0; i < 10; i++) {
        double similarity = 0.75 + (rand() % 10) / 100.0;  // 0.75-0.84
        scorer.recordProposalSimilarity(user1, "PROP_" + to_string(i), 
                                       similarity, "2024-11-09", "TECH");
    }
    
    // User 2: Erratic proposals (low consistency)
    string user2 = "USER_ERRATIC";
    cout << "User 2 (" << user2 << "): Erratic proposal quality\n";
    for (int i = 0; i < 10; i++) {
        double similarity = (rand() % 80) / 100.0;  // 0.0-0.79 (wide range)
        scorer.recordProposalSimilarity(user2, "PROP_" + to_string(i + 10), 
                                       similarity, "2024-11-09", "TECH");
    }
    
    // User 3: New user (default consistency)
    string user3 = "USER_NEW";
    cout << "User 3 (" << user3 << "): New user with few proposals\n";
    for (int i = 0; i < 2; i++) {
        scorer.recordProposalSimilarity(user3, "PROP_" + to_string(i + 20), 
                                       0.6, "2024-11-09", "ENV");
    }
    
    printSubHeader("Consistency Metrics");
    
    auto metrics1 = scorer.getUserConsistencyMetrics(user1);
    cout << "User 1 Metrics:\n";
    cout << "  Mean Similarity: " << fixed << setprecision(3) << metrics1.meanSimilarity << "\n";
    cout << "  Std Deviation: " << metrics1.stdDevSimilarity << "\n";
    cout << "  Consistency Score: " << metrics1.consistencyScore << " ⭐ (HIGH)\n";
    cout << "  Interpretation: Stable, reliable contributor\n\n";
    
    auto metrics2 = scorer.getUserConsistencyMetrics(user2);
    cout << "User 2 Metrics:\n";
    cout << "  Mean Similarity: " << metrics2.meanSimilarity << "\n";
    cout << "  Std Deviation: " << metrics2.stdDevSimilarity << "\n";
    cout << "  Consistency Score: " << metrics2.consistencyScore << " ⚠ (LOW)\n";
    cout << "  Interpretation: Erratic, inconsistent proposals\n\n";
    
    auto metrics3 = scorer.getUserConsistencyMetrics(user3);
    cout << "User 3 Metrics:\n";
    cout << "  Consistency Score: " << metrics3.consistencyScore << " (NEUTRAL - New user)\n\n";
    
    printSubHeader("Weighted Ranking Scores");
    
    // Calculate weighted scores for sample proposals
    auto score1 = scorer.calculateWeightedScore("PROP_NEW_1", user1, 0.8, 0.9, 1.0);
    cout << "Proposal by User 1 (consistent):\n";
    cout << "  Relevance: " << score1.relevanceScore << "\n";
    cout << "  Consistency: " << score1.consistencyScore << " ⭐\n";
    cout << "  Trust: " << score1.trustScore << "\n";
    cout << "  Recency: " << score1.recencyScore << "\n";
    cout << "  → Final Score: " << score1.finalWeightedScore << "\n\n";
    
    auto score2 = scorer.calculateWeightedScore("PROP_NEW_2", user2, 0.8, 0.9, 1.0);
    cout << "Proposal by User 2 (erratic):\n";
    cout << "  Relevance: " << score2.relevanceScore << "\n";
    cout << "  Consistency: " << score2.consistencyScore << " ⚠\n";
    cout << "  Trust: " << score2.trustScore << "\n";
    cout << "  Recency: " << score2.recencyScore << "\n";
    cout << "  → Final Score: " << score2.finalWeightedScore << "\n\n";
    
    cout << "Impact: User 1's proposal scores " 
         << fixed << setprecision(2) 
         << ((score1.finalWeightedScore / score2.finalWeightedScore - 1) * 100)
         << "% higher due to consistency bonus!\n";
    
    cout << scorer.getConsistencyStatistics();
}

void demonstrateAntiAbuseEngine() {
    printHeader("ANTI-ABUSE ENGINE: BOT & COLLUSION DETECTION");
    
    AntiAbuseEngine engine(30.0, 200.0, 60);
    
    cout << engine.getConfiguration() << "\n";
    
    printSubHeader("Simulating Voting Patterns");
    
    auto now = chrono::system_clock::now();
    
    // Normal user behavior
    string normalUser = "USER_NORMAL";
    cout << "Normal User: Votes at regular intervals\n";
    for (int i = 0; i < 5; i++) {
        auto timestamp = now + chrono::seconds(i * 15);  // 15 seconds apart
        engine.recordVoteEvent(normalUser, "PROP_" + to_string(i), 
                              timestamp, "IP_123", "DEV_ABC");
    }
    
    // Bot-like behavior
    string botUser = "USER_BOT";
    cout << "Bot User: Votes very rapidly\n";
    for (int i = 0; i < 50; i++) {
        auto timestamp = now + chrono::milliseconds(i * 100);  // 100ms apart
        engine.recordVoteEvent(botUser, "PROP_" + to_string(i), 
                              timestamp, "IP_999", "DEV_BOT");
    }
    
    // Collusion group
    vector<string> collusionGroup = {"USER_COL_1", "USER_COL_2", "USER_COL_3"};
    cout << "Collusion Group: Users always vote on same proposals\n";
    for (int i = 0; i < 10; i++) {
        string proposalId = "PROP_COL_" + to_string(i);
        for (const auto& user : collusionGroup) {
            auto timestamp = now + chrono::seconds(i * 5);
            engine.recordVoteEvent(user, proposalId, timestamp, 
                                  "IP_COL", "DEV_" + user);
        }
    }
    
    printSubHeader("Bot Detection Results");
    
    auto botResults = engine.detectAllBots();
    cout << "Found " << botResults.size() << " suspicious users:\n\n";
    
    for (const auto& result : botResults) {
        cout << result.userId << ":\n";
        cout << "  Bot Likelihood: " << fixed << setprecision(2) 
             << result.botLikelihood * 100 << "%\n";
        cout << "  Voting Velocity: " << result.votingVelocity << " votes/min\n";
        cout << "  Avg Inter-vote Gap: " << result.avgInterVoteGapMs << " ms\n";
        cout << "  Device Diversity: " << result.deviceDiversity << "\n";
        cout << "  Status: " << (result.isSuspicious ? "🚨 SUSPICIOUS" : "✓ Normal") << "\n";
        cout << "  Reason: " << result.reason << "\n\n";
    }
    
    printSubHeader("Collusion Detection Results");
    
    auto collusionResults = engine.detectCollusion();
    cout << "Found " << collusionResults.size() << " suspicious groups:\n\n";
    
    for (const auto& result : collusionResults) {
        cout << "Group of " << result.userGroup.size() << " users:\n";
        cout << "  Users: ";
        for (const auto& user : result.userGroup) {
            cout << user << " ";
        }
        cout << "\n";
        cout << "  Collusion Score: " << fixed << setprecision(2) 
             << result.collusionScore * 100 << "%\n";
        cout << "  Co-votes: " << result.coVoteCount << "\n";
        cout << "  Status: " << (result.isSuspicious ? "🚨 SUSPICIOUS" : "✓ Normal") << "\n";
        cout << "  Description: " << result.description << "\n\n";
    }
    
    printSubHeader("User Credibility Scores");
    
    engine.calculateAllCredibilityScores();
    
    for (const auto& user : {normalUser, botUser, collusionGroup[0]}) {
        double trustScore = engine.getUserTrustScore(user);
        cout << user << ": Trust Score = " << fixed << setprecision(3) 
             << trustScore << " ";
        if (trustScore > 0.7) cout << "✓ Trusted\n";
        else if (trustScore > 0.4) cout << "⚠ Questionable\n";
        else cout << "🚨 Untrusted\n";
    }
    
    cout << "\n" << engine.performSecurityScan();
    cout << engine.getSecurityStatistics();
}

void demonstrateEnsembleModels() {
    printHeader("ENSEMBLE ML MODELS");
    
    printSubHeader("Generating Training Data");
    
    // Create synthetic training data
    vector<FeatureVector> trainingData;
    
    // Class 1: High priority proposals (many votes)
    for (int i = 0; i < 20; i++) {
        FeatureVector fv("TRAIN_HIGH_" + to_string(i));
        fv.features["vote_count"] = 30 + rand() % 20;
        fv.features["title_length"] = 50 + rand() % 30;
        fv.features["description_length"] = 200 + rand() % 100;
        fv.textTokens = {"urgent", "important", "critical", "priority", "action"};
        fv.groundTruthLabel = "high_priority";
        trainingData.push_back(fv);
    }
    
    // Class 2: Low priority proposals (few votes)
    for (int i = 0; i < 20; i++) {
        FeatureVector fv("TRAIN_LOW_" + to_string(i));
        fv.features["vote_count"] = 1 + rand() % 5;
        fv.features["title_length"] = 20 + rand() % 20;
        fv.features["description_length"] = 50 + rand() % 50;
        fv.textTokens = {"minor", "suggestion", "idea", "maybe", "consider"};
        fv.groundTruthLabel = "low_priority";
        trainingData.push_back(fv);
    }
    
    cout << "Generated " << trainingData.size() << " training samples\n";
    cout << "  - High priority: 20 samples\n";
    cout << "  - Low priority: 20 samples\n\n";
    
    printSubHeader("Training Individual Models");
    
    // Train Naive Bayes
    cout << "Training Naive Bayes classifier...\n";
    NaiveBayesClassifier nb(1.0);
    nb.train(trainingData);
    cout << nb.getModelInfo() << "\n";
    
    // Train Random Forest
    cout << "Training Random Forest classifier...\n";
    RandomForestClassifier rf(10, 5, 2, 0.7);
    rf.train(trainingData);
    cout << rf.getModelInfo() << "\n";
    
    printSubHeader("Training Ensemble");
    
    EnsembleClassifier ensemble("weighted");
    ensemble.configureModels(true, true);
    ensemble.train(trainingData);
    cout << ensemble.getEnsembleInfo() << "\n";
    
    printSubHeader("Making Predictions");
    
    // Test sample 1: Should be high priority
    FeatureVector test1("TEST_1");
    test1.features["vote_count"] = 35;
    test1.features["title_length"] = 60;
    test1.features["description_length"] = 250;
    test1.textTokens = {"critical", "urgent", "important"};
    
    cout << "Test Proposal 1 (Expected: high_priority):\n";
    cout << "  Vote count: " << test1.features["vote_count"] << "\n";
    cout << "  Title length: " << test1.features["title_length"] << "\n";
    cout << "  Keywords: critical, urgent, important\n\n";
    
    auto prediction1 = ensemble.predict(test1);
    cout << "  Ensemble Prediction: " << prediction1.finalLabel 
         << " (confidence: " << fixed << setprecision(2) 
         << prediction1.confidence * 100 << "%)\n";
    cout << "  Model votes:\n";
    for (const auto& pair : prediction1.modelVotes) {
        cout << "    " << pair.first << ": " << pair.second << "\n";
    }
    cout << "\n";
    
    // Test sample 2: Should be low priority
    FeatureVector test2("TEST_2");
    test2.features["vote_count"] = 2;
    test2.features["title_length"] = 25;
    test2.features["description_length"] = 60;
    test2.textTokens = {"suggestion", "maybe", "idea"};
    test2.groundTruthLabel = "low_priority";
    
    cout << "Test Proposal 2 (Expected: low_priority):\n";
    cout << "  Vote count: " << test2.features["vote_count"] << "\n";
    cout << "  Title length: " << test2.features["title_length"] << "\n";
    cout << "  Keywords: suggestion, maybe, idea\n\n";
    
    auto prediction2 = ensemble.predict(test2);
    cout << "  Ensemble Prediction: " << prediction2.finalLabel 
         << " (confidence: " << prediction2.confidence * 100 << "%)\n";
    cout << "  Model votes:\n";
    for (const auto& pair : prediction2.modelVotes) {
        cout << "    " << pair.first << ": " << pair.second << "\n";
    }
    cout << "\n";
    
    printSubHeader("Individual Model Predictions");
    
    auto individualPreds = ensemble.getIndividualPredictions(test1);
    for (const auto& pair : individualPreds) {
        cout << pair.first << ": " << pair.second.label 
             << " (" << fixed << setprecision(2) 
             << pair.second.confidence * 100 << "%)\n";
    }
}

void demonstrateStreamProcessing() {
    printHeader("STREAM PROCESSING ARCHITECTURE");
    
    cout << StreamProcessor::getProductionInfo() << "\n";
    
    printSubHeader("Simulating Event Streaming");
    
    StreamProcessor stream(1000);
    
    // Set up event handlers
    stream.setVoteHandler([](const StreamEvent& event) {
        cout << "  [Vote Handler] Processing: " << event.eventId << "\n";
    });
    
    stream.setProposalHandler([](const StreamEvent& event) {
        cout << "  [Proposal Handler] Processing: " << event.eventId << "\n";
    });
    
    stream.start();
    
    // Produce some events
    cout << "Producing events...\n";
    for (int i = 0; i < 5; i++) {
        StreamEvent voteEvent("vote", "{user_id: USER_" + to_string(i) + "}");
        voteEvent.eventId = "VOTE_" + to_string(i);
        stream.produce(voteEvent);
    }
    
    StreamEvent propEvent("proposal", "{title: 'New Proposal'}");
    propEvent.eventId = "PROP_1";
    stream.produce(propEvent);
    
    cout << "\nQueue size: " << stream.getQueueSize() << "\n";
    cout << "Backpressure: " << (stream.hasBackpressure() ? "Yes" : "No") << "\n\n";
    
    // Consume events
    cout << "Consuming events...\n";
    int processed = stream.consume(10);
    cout << "\nProcessed " << processed << " events\n";
    cout << "Remaining queue size: " << stream.getQueueSize() << "\n\n";
    
    stream.stop();
}

void demonstrateIntegration() {
    printHeader("INTEGRATED CROWDDECISION SYSTEM");
    
    cout << "Demonstrating full system integration:\n\n";
    
    // Create components
    VotingSystem votingSystem;
    ConsistencyScorer consistencyScorer;
    AntiAbuseEngine antiAbuse;
    
    printSubHeader("Step 1: User Registration");
    
    auto user1 = votingSystem.registerUser("Alice");
    auto user2 = votingSystem.registerUser("Bob");
    auto user3 = votingSystem.registerUser("Charlie");
    
    cout << "Registered 3 users\n\n";
    
    printSubHeader("Step 2: Create Proposals");
    
    auto prop1 = votingSystem.createProposal(
        "Implement AI-Powered Healthcare System",
        "Use machine learning for early disease detection",
        user1
    );
    
    auto prop2 = votingSystem.createProposal(
        "Green Energy Initiative",
        "Transition to 100% renewable energy",
        user2
    );
    
    cout << "Created 2 proposals\n\n";
    
    printSubHeader("Step 3: Cast Votes & Track");
    
    auto now = chrono::system_clock::now();
    
    // Normal voting
    votingSystem.castVote(user2, prop1);
    antiAbuse.recordVoteEvent(user2, prop1, now, "IP_1", "DEV_1");
    
    votingSystem.castVote(user3, prop1);
    antiAbuse.recordVoteEvent(user3, prop1, now + chrono::seconds(5), "IP_2", "DEV_2");
    
    votingSystem.castVote(user1, prop2);
    antiAbuse.recordVoteEvent(user1, prop2, now + chrono::seconds(10), "IP_1", "DEV_1");
    
    cout << "Cast 3 votes\n\n";
    
    printSubHeader("Step 4: Calculate Consistency");
    
    consistencyScorer.recordProposalSimilarity(user1, prop1, 0.85, "2024-11-09", "TECH");
    consistencyScorer.recordProposalSimilarity(user2, prop2, 0.78, "2024-11-09", "ENV");
    
    auto metrics1 = consistencyScorer.getUserConsistencyMetrics(user1);
    cout << "Alice's consistency score: " << fixed << setprecision(3) 
         << metrics1.consistencyScore << "\n";
    
    auto metrics2 = consistencyScorer.getUserConsistencyMetrics(user2);
    cout << "Bob's consistency score: " << metrics2.consistencyScore << "\n\n";
    
    printSubHeader("Step 5: Security Scan");
    
    cout << antiAbuse.performSecurityScan();
    
    printSubHeader("Step 6: Calculate Trust Scores");
    
    antiAbuse.calculateAllCredibilityScores();
    
    cout << "Alice's trust score: " << antiAbuse.getUserTrustScore(user1) << "\n";
    cout << "Bob's trust score: " << antiAbuse.getUserTrustScore(user2) << "\n";
    cout << "Charlie's trust score: " << antiAbuse.getUserTrustScore(user3) << "\n\n";
    
    printSubHeader("Step 7: Final Weighted Rankings");
    
    // Calculate final weighted scores
    double relevance1 = 0.85;
    double trust1 = antiAbuse.getUserTrustScore(user1);
    double consistency1 = consistencyScorer.getUserConsistencyScore(user1);
    
    auto finalScore1 = consistencyScorer.calculateWeightedScore(
        prop1, user1, relevance1, trust1, 1.0);
    
    cout << "Proposal 1 Final Score: " << fixed << setprecision(3) 
         << finalScore1.finalWeightedScore << "\n";
    cout << "  Components:\n";
    cout << "    Relevance: " << finalScore1.relevanceScore << " × 0.55\n";
    cout << "    Consistency: " << finalScore1.consistencyScore << " × 0.25\n";
    cout << "    Trust: " << finalScore1.trustScore << " × 0.15\n";
    cout << "    Recency: " << finalScore1.recencyScore << " × 0.05\n";
    
    cout << "\n✅ Full CrowdDecision pipeline demonstrated!\n";
}

int main() {
    srand(time(nullptr));
    
    printHeader("CROWDDECISION - COMPREHENSIVE DEMO");
    
    cout << R"(
This demonstration showcases the complete CrowdDecision architecture:

1. Weight-Based Consistency & Stability Scoring
2. Anti-Abuse Engine (Bot & Collusion Detection)
3. Ensemble ML Models (Naive Bayes + Random Forest)
4. Stream Processing Architecture (Conceptual)
5. Full System Integration

Press Enter to continue...
)";
    
    cin.get();
    
    try {
        demonstrateConsistencyScoring();
        cout << "\nPress Enter to continue to Anti-Abuse Engine...\n";
        cin.get();
        
        demonstrateAntiAbuseEngine();
        cout << "\nPress Enter to continue to Ensemble Models...\n";
        cin.get();
        
        demonstrateEnsembleModels();
        cout << "\nPress Enter to continue to Stream Processing...\n";
        cin.get();
        
        demonstrateStreamProcessing();
        cout << "\nPress Enter to see Full System Integration...\n";
        cin.get();
        
        demonstrateIntegration();
        
        printHeader("DEMO COMPLETE");
        cout << R"(
✅ All CrowdDecision features demonstrated successfully!

Key Takeaways:
• Consistency scoring rewards stable, reliable contributors
• Anti-abuse engine detects bots and collusion in real-time
• Ensemble models combine multiple ML approaches for better accuracy
• Streaming architecture enables real-time processing at scale
• All components integrate seamlessly for production deployment

For more information, see:
- CROWDDECISION_ARCHITECTURE.md
- Implementation files: ConsistencyScorer.*, AntiAbuseEngine.*, EnsembleModels.*
)";
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
