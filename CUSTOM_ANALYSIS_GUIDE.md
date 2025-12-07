# Custom Proposal Analysis Guide

## 🎯 Use Your Own Proposals!

Instead of using demo data, you can now analyze **your own proposals** with all the advanced analytics features.

---

## 🚀 How to Run

```bash
make custom
```

This will start an **interactive program** where you input your own data.

---

## 📝 Step-by-Step Process

### Step 1: Register Users
```
How many users do you want to register? 3
Enter username 1: Alice
Enter username 2: Bob
Enter username 3: Charlie
```

### Step 2: Create Proposals
```
How many proposals do you want to create? 2

--- Proposal 1 ---
Title: AI-Powered Healthcare System
Description: Use machine learning for early disease detection
Creator (enter user number 1-3): 1

--- Proposal 2 ---
Title: Green Energy Initiative
Description: Build solar and wind farms for renewable energy
Creator (enter user number 1-3): 2
```

### Step 3: Cast Votes (Optional)
```
Do you want to cast votes? (y/n): y
How many votes to cast? 3

Vote 1:
  User (1-3): 1
  Proposal (1-2): 2

Vote 2:
  User (1-3): 2
  Proposal (1-2): 1

Vote 3:
  User (1-3): 3
  Proposal (1-2): 1
```

### Step 4: Automatic Analytics

The program will automatically run:

1. **Similarity Analysis** - Compare all proposals
2. **Topic Analysis** - Identify topics (Tech, Environment, etc.)
3. **Classification** - Classify as high/medium/low priority
4. **Decision Ranking** - Rank by selected topic
5. **Rank & Percentile** - Overall rankings
6. **Normalization** - MinMax and Z-Score

---

## 📊 What You'll Get

### 1. Similarity Analysis
```
Pairwise Similarity (Jaccard + Cosine):

Proposal 1 ↔ Proposal 2: 0.156 (Moderate similarity)
```

### 2. Topic Analysis
```
Proposal Topics:

Proposal 1: AI-Powered Healthcare System
  Topics: Technology Healthcare

Proposal 2: Green Energy Initiative
  Topics: Environment
```

### 3. Classification
```
Classification Results:

Proposal 1: AI-Powered Healthcare System
  Votes: 2
  Classification: low_priority
  Confidence: 99.8%
```

### 4. Decision Ranking
```
Available Topics:
  1. Technology (TECH)
  2. Environment (ENV)
  3. Education (EDU)
  4. Healthcare (HEALTH)
  5. Economy (ECON)

Select topic for ranking (1-5): 1

Rankings for Technology topic:
Rank | Title                          | Score | Percentile
   1 | AI-Powered Healthcare System   | 1.000 |  100.0%
```

### 5. Rank & Percentile
```
Overall Rankings:
Rank | Percentile | Votes | Title
   1 |      50.0% |     2 | AI-Powered Healthcare System
   2 |      50.0% |     1 | Green Energy Initiative
```

### 6. Score Normalization
```
Original Vote Counts: 2 1
MinMax Normalized [0-1]: 1.000 0.000
Z-Score Standardized: 0.707 -0.707
```

---

## 💡 Example Session

```bash
$ make custom

╔════════════════════════════════════════════════════════════════════╗
║     CUSTOM PROPOSAL ANALYSIS                                       ║
║     Analyze Your Own Proposals with Advanced Analytics             ║
╚════════════════════════════════════════════════════════════════════╝

=== STEP 1: Register Users ===
How many users do you want to register? 2
Enter username 1: Alice
  ✓ Registered: Alice (ID: USER_123456)
Enter username 2: Bob
  ✓ Registered: Bob (ID: USER_789012)

=== STEP 2: Create Proposals ===
How many proposals do you want to create? 2

--- Proposal 1 ---
Title: Smart City IoT Platform
Description: Deploy sensors and AI for urban management
Creator (enter user number 1-2): 1
  ✓ Created proposal: Smart City IoT Platform

--- Proposal 2 ---
Title: Climate Action Plan
Description: Reduce carbon emissions and promote sustainability
Creator (enter user number 1-2): 2
  ✓ Created proposal: Climate Action Plan

=== STEP 3: Cast Votes (Optional) ===
Do you want to cast votes? (y/n): y
How many votes to cast? 2

Vote 1:
  User (1-2): 1
  Proposal (1-2): 2
  ✓ Vote cast successfully

Vote 2:
  User (1-2): 2
  Proposal (1-2): 1
  ✓ Vote cast successfully

[... Analytics results follow ...]
```

---

## 🎯 All Features Applied to YOUR Data

✅ **Jaccard Index** - Measures set similarity between your proposals  
✅ **Cosine Similarity** - Measures vector similarity of your text  
✅ **Topic Analysis** - Identifies topics in your proposals  
✅ **Binary Search** - Fast keyword lookup in topics  
✅ **Logistic Regression** - Classifies your proposals  
✅ **Time-Based Filtering** - Applies time decay to your data  
✅ **Decision Ranking** - Ranks your proposals by topic  
✅ **Similarity Matrix** - Shows relationships between your proposals  
✅ **Rank & Percentile** - Calculates rankings for your proposals  
✅ **MinMax Normalization** - Normalizes your scores  
✅ **Z-Score Standardization** - Standardizes your data  

---

## 🔧 Tips

### For Better Results:

1. **Use Descriptive Titles**: Help topic detection
   - ✓ "AI-Powered Healthcare System"
   - ✗ "Proposal 1"

2. **Add Detailed Descriptions**: Improves similarity analysis
   - ✓ "Use machine learning algorithms for early disease detection and diagnosis"
   - ✗ "Healthcare stuff"

3. **Cast Multiple Votes**: Better classification and ranking
   - More votes = more accurate priority classification

4. **Use Relevant Keywords**: Helps topic matching
   - Technology: ai, machine, learning, software, digital
   - Environment: climate, green, renewable, sustainability
   - Education: school, learning, student, academic
   - Healthcare: medical, health, hospital, treatment
   - Economy: business, financial, market, investment

---

## 🆚 Comparison: Demo vs Custom

| Feature | Demo (`make advanced`) | Custom (`make custom`) |
|---------|----------------------|----------------------|
| Data Source | Pre-defined samples | **Your own input** |
| Proposals | 5 fixed proposals | **Any number you want** |
| Users | 5 fixed users | **Any number you want** |
| Votes | 12 pre-cast votes | **You decide** |
| Interaction | Watch only | **Fully interactive** |
| Use Case | Learn features | **Analyze real data** |

---

## 📚 What Each Analysis Tells You

### Similarity Analysis
- **High (>0.3)**: Proposals are very similar, might be duplicates
- **Moderate (0.15-0.3)**: Some overlap, related topics
- **Low (<0.15)**: Different topics, independent proposals

### Topic Analysis
- Shows which domain each proposal belongs to
- Helps organize and categorize proposals
- Useful for filtering by interest area

### Classification
- **High Priority**: Many votes, high engagement
- **Medium Priority**: Moderate interest
- **Low Priority**: New or less popular

### Decision Ranking
- Ranks proposals by relevance to a specific topic
- Considers recency and engagement
- Helps prioritize decisions in specific domains

### Rank & Percentile
- **Rank**: Absolute position (1 = best)
- **Percentile**: Relative position (90% = top 10%)
- Shows overall standing

### Normalization
- **MinMax**: Easy comparison on 0-1 scale
- **Z-Score**: Shows how many standard deviations from mean
- Useful for fair comparison across different metrics

---

## 🎓 Learning Objectives

By using this tool, you'll understand:

1. How **Jaccard and Cosine similarity** work on real text
2. How **topic analysis** categorizes content
3. How **logistic regression** classifies data
4. How **ranking algorithms** combine multiple factors
5. How **normalization** makes fair comparisons
6. How **data structures** (HashMaps, heaps) improve performance

---

## 🚀 Quick Start Commands

```bash
# Clean previous builds
make clean

# Run with your own proposals
make custom

# Run demo to see examples first
make advanced

# See all available commands
make help
```

---

## 💡 Pro Tips

1. **Start Small**: Try 2-3 proposals first to understand the output
2. **Compare Similar**: Create similar proposals to see high similarity scores
3. **Compare Different**: Create different proposals to see low similarity scores
4. **Add Votes**: Cast votes to see classification in action
5. **Try Different Topics**: Select different topics for ranking to see how relevance changes

---

## ✅ Success Checklist

- [ ] Compiled successfully with `make custom`
- [ ] Registered at least 2 users
- [ ] Created at least 2 proposals
- [ ] Cast some votes (optional but recommended)
- [ ] Saw similarity analysis results
- [ ] Saw topic analysis results
- [ ] Saw classification results
- [ ] Selected a topic for ranking
- [ ] Saw rank & percentile results
- [ ] Understood normalization output

---

## 🎉 You're Ready!

Now you can analyze **any proposals** with the same advanced analytics that power recommendation systems, search engines, and ML platforms!

**Run it now:**
```bash
make custom
```
