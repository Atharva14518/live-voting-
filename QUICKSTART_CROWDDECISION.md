# CrowdDecision - Quick Start Guide

## 🚀 Get Started in 3 Steps

### 1. Build the Project
```bash
cd "ds cp 3"
make clean
make crowddecision
```

### 2. Run the Demo
```bash
./crowddecision_demo
```

### 3. Explore Features
The demo will walk you through:
- Consistency & Stability Scoring
- Bot & Collusion Detection
- Ensemble ML Models
- Stream Processing
- Full System Integration

Press Enter at each step to continue.

---

## 📚 What's Implemented

### New Modules (CrowdDecision)

| Module | File | Description |
|--------|------|-------------|
| **Consistency Scorer** | `ConsistencyScorer.h/cpp` | Rewards stable contributors |
| **Anti-Abuse Engine** | `AntiAbuseEngine.h/cpp` | Detects bots & collusion |
| **Ensemble Models** | `EnsembleModels.h/cpp` | ML classification (NB + RF) |
| **Stream Processor** | `StreamProcessor.h/cpp` | Event streaming architecture |

### Key Features

✅ **Consistency Scoring**: `1 / (1 + σ)` formula  
✅ **Bot Detection**: Velocity analysis (votes/min)  
✅ **Collusion Detection**: Co-voting graph analysis  
✅ **User Credibility**: Multi-factor trust scoring  
✅ **Naive Bayes**: Fast text classification  
✅ **Random Forest**: Ensemble of 10 decision trees  
✅ **Weighted Ranking**: α=0.55, β=0.25, γ=0.15, δ=0.05  

---

## 🎯 Core Formulas

### Consistency Score
```
consistency_score = 1 / (1 + stdDev(user_similarities))
```

### Weighted Ranking
```
final_score = 0.55 × relevance 
            + 0.25 × consistency 
            + 0.15 × trust 
            + 0.05 × recency
```

### Bot Likelihood
```
bot_likelihood = 0.4 × velocity_score 
               + 0.4 × gap_score 
               + 0.2 × device_score
```

---

## 💻 Code Examples

### Example 1: Consistency Scoring
```cpp
#include "ConsistencyScorer.h"

ConsistencyScorer scorer(50);  // 50-proposal rolling window

// Record user's proposal similarity
scorer.recordProposalSimilarity(
    "USER_123",      // user ID
    "PROP_456",      // proposal ID
    0.85,            // similarity score (0-1)
    "2024-11-09",    // timestamp
    "TECH"           // topic
);

// Get consistency score
double consistency = scorer.getUserConsistencyScore("USER_123");
// Returns: 0.0 to 1.0 (higher = more consistent)

// Calculate weighted score
auto weighted = scorer.calculateWeightedScore(
    "PROP_456", "USER_123", 
    0.85,  // relevance
    0.90,  // trust
    1.0    // recency
);
cout << "Final Score: " << weighted.finalWeightedScore;
```

### Example 2: Bot Detection
```cpp
#include "AntiAbuseEngine.h"

AntiAbuseEngine engine(30.0, 200.0, 60);  // velocity, delta, window

// Record vote events
auto now = chrono::system_clock::now();
engine.recordVoteEvent(
    "USER_123",      // user ID
    "PROP_456",      // proposal ID
    now,             // timestamp
    "IP_HASH_XYZ",   // hashed IP
    "DEV_HASH_ABC"   // hashed device
);

// Detect bots
auto botResult = engine.detectBot("USER_123");
cout << "Bot Likelihood: " << botResult.botLikelihood * 100 << "%\n";
cout << "Velocity: " << botResult.votingVelocity << " votes/min\n";

// Get trust score
double trust = engine.getUserTrustScore("USER_123");
```

### Example 3: Ensemble ML
```cpp
#include "EnsembleModels.h"

// Create training data
vector<FeatureVector> trainingData;
// ... populate training data ...

// Train ensemble
EnsembleClassifier ensemble("weighted");
ensemble.train(trainingData);

// Predict
FeatureVector testSample;
testSample.features["vote_count"] = 35;
testSample.features["title_length"] = 60;
testSample.textTokens = {"critical", "urgent"};

auto prediction = ensemble.predict(testSample);
cout << "Predicted: " << prediction.finalLabel << "\n";
cout << "Confidence: " << prediction.confidence * 100 << "%\n";
```

---

## 🔧 Configuration

### Adjust Consistency Weights
```cpp
ConsistencyScorer::configureWeights(
    0.55,  // α - relevance weight
    0.25,  // β - consistency weight
    0.15,  // γ - trust weight
    0.05   // δ - recency weight
);
```

### Adjust Bot Detection Thresholds
```cpp
engine.configureThresholds(
    30.0,   // velocity threshold (votes/min)
    200.0,  // inter-vote gap (ms)
    0.7,    // collusion threshold
    0.7     // bot likelihood threshold
);
```

### Configure Random Forest
```cpp
RandomForestClassifier rf(
    10,    // number of trees
    5,     // max depth
    2,     // min samples to split
    0.7    // feature sampling ratio
);
```

---

## 📊 Understanding the Output

### Consistency Metrics
```
Mean Similarity: 0.800    # Average proposal quality
Std Deviation: 0.050      # Consistency (lower = more stable)
Consistency Score: 0.952  # Final score (higher = better)
```

**Interpretation**:
- **High consistency (>0.7)**: Reliable, stable contributor
- **Medium (0.3-0.7)**: Average contributor
- **Low (<0.3)**: Erratic, inconsistent proposals

### Bot Detection
```
Bot Likelihood: 85%
Voting Velocity: 45 votes/min
Avg Inter-vote Gap: 150 ms
Status: 🚨 SUSPICIOUS
```

**Red Flags**:
- Velocity > 30 votes/min
- Gap < 200ms between votes
- Single device/IP for many votes

### Trust Score
```
Trust Score: 0.75 ✓ Trusted
```

**Ranges**:
- **0.7-1.0**: Trusted
- **0.4-0.7**: Questionable
- **0.0-0.4**: Untrusted

---

## 🏗️ Architecture Overview

```
User Input → Voting System → Stream Processor
                ↓
         Consistency Scorer ← Proposal History
                ↓
         Anti-Abuse Engine ← Vote Events
                ↓
         Ensemble Classifier ← Features
                ↓
         Weighted Ranking → Top Proposals
```

---

## 📖 Documentation Files

| File | Purpose |
|------|---------|
| **CROWDDECISION_ARCHITECTURE.md** | Complete specification (500+ lines) |
| **INTEGRATION_COMPLETE.md** | Implementation summary |
| **README.md** | Project overview |
| **IMPLEMENTATION_SUMMARY.md** | Advanced analytics details |
| **ADVANCED_FEATURES.md** | NLP features documentation |

---

## 🎮 Other Make Targets

```bash
make                  # Build main system
make run              # Run main voting system
make advanced         # Advanced analytics demo
make intelligence     # AI features demo
make custom           # Interactive proposal analyzer
make clean            # Clean build files
make help             # Show all targets
```

---

## 🐛 Troubleshooting

### Compilation Errors
```bash
# Clean and rebuild
make clean
make crowddecision
```

### Missing Headers
Ensure you're in the correct directory:
```bash
cd "ds cp 3"
ls -la *.h *.cpp
```

### Demo Not Running
Check executable permissions:
```bash
chmod +x crowddecision_demo
./crowddecision_demo
```

---

## 🎯 Next Steps

1. **Run the demo**: `make crowddecision`
2. **Read architecture**: `CROWDDECISION_ARCHITECTURE.md`
3. **Explore code**: Start with `ConsistencyScorer.h`
4. **Customize config**: Adjust weights and thresholds
5. **Integrate**: Use modules in your own code

---

## 💡 Tips

- **Start simple**: Run the demo first to understand each component
- **Read output carefully**: The demo explains what's happening
- **Check statistics**: Use `getStatistics()` methods for insights
- **Experiment**: Adjust thresholds to see behavior changes
- **Integrate gradually**: Add one module at a time to your code

---

## 🏆 Key Takeaways

✅ **Consistency rewards stability** - Users with consistent proposals rank higher  
✅ **Anti-abuse is automatic** - Bots and collusion detected in real-time  
✅ **ML improves accuracy** - Ensemble models combine multiple algorithms  
✅ **Everything integrates** - All components work together seamlessly  

---

## 📞 Need Help?

- Check `CROWDDECISION_ARCHITECTURE.md` for detailed specs
- Read `INTEGRATION_COMPLETE.md` for implementation details
- Review code comments in header files
- Run `make help` for available commands

---

**Ready to start?**

```bash
make crowddecision
```

🎉 **Enjoy exploring CrowdDecision!**
