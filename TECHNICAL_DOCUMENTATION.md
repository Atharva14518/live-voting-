# CrowdDecision - Technical Documentation
## Data Structures, Algorithms & Machine Learning Models

---

## 1. DATA STRUCTURES USED

### 1.1 Max Heap (Priority Queue)
**File:** `server/src/core/MaxHeap.ts`

**What is it?**
A Max Heap is a complete binary tree where the parent node is always greater than or equal to its children. The root element is always the maximum.

**Why used?**
To efficiently maintain proposals ranked by vote count. When users vote, we need to quickly update rankings and retrieve the top proposals.

**Operations & Time Complexity:**
| Operation | Time Complexity | Description |
|-----------|-----------------|-------------|
| Insert | O(log n) | Add a new proposal |
| Extract Max | O(log n) | Get the highest-voted proposal |
| Peek Max | O(1) | View top proposal without removing |
| Update | O(n) | Update vote count and re-heapify |
| Get All (sorted) | O(n log n) | Return all proposals in ranked order |

**Implementation:**
```
        [52 votes]           <- Root (Maximum)
       /          \
  [45 votes]    [38 votes]
   /     \
[35]    [28]
```

---

### 1.2 Hash Map (Dictionary)
**File:** `server/src/routes/api.ts`

**What is it?**
A key-value data structure that provides constant-time lookups using a hash function.

**Why used?**
- Store users by ID for quick authentication
- Store proposals by ID for fast retrieval
- Store sessions for login management
- Rate limiting (IP -> request count)

**Time Complexity:**
| Operation | Time Complexity |
|-----------|-----------------|
| Insert | O(1) average |
| Lookup | O(1) average |
| Delete | O(1) average |
| Iterate | O(n) |

**Example Usage:**
```typescript
const users = new Map<string, User>();
const proposals = new Map<string, Proposal>();
```

---

### 1.3 Linked List (Implicit in Audit Log)
**File:** `server/src/core/AuditLog.ts`

**What is it?**
A sequence of entries where each entry contains a reference (hash) to the previous entry.

**Why used?**
Creates a tamper-evident blockchain-like audit trail where any modification is detectable.

**Time Complexity:**
| Operation | Time Complexity |
|-----------|-----------------|
| Append Entry | O(1) |
| Verify Integrity | O(n) |
| Get All Logs | O(n) |

---

### 1.4 Set
**File:** `server/src/core/AdvancedAntiBot.ts`

**What is it?**
A collection of unique elements with no duplicates.

**Why used?**
Track suspicious/blocked IPs without duplicates.

**Time Complexity:**
| Operation | Time Complexity |
|-----------|-----------------|
| Add | O(1) |
| Check exists | O(1) |
| Delete | O(1) |

---

## 2. ALGORITHMS USED

### 2.1 SHA-256 Hashing Algorithm
**File:** `server/src/core/AuditLog.ts`, `server/src/core/AuthService.ts`

**What is it?**
A cryptographic hash function that produces a 256-bit (32-byte) hash value.

**Why used?**
1. **Password Security:** Hash passwords before storing (never store plain text)
2. **Audit Log Integrity:** Chain each log entry's hash to the previous one

**Time Complexity:** O(n) where n is the input length

**How it works in Audit Log:**
```
Entry 1: hash("action1 + data1 + 0") = "abc123..."
Entry 2: hash("action2 + data2 + abc123...") = "def456..."
Entry 3: hash("action3 + data3 + def456...") = "ghi789..."
```
If anyone modifies Entry 1, all subsequent hashes become invalid!

---

### 2.2 Heapify Algorithm (Sift Up / Sift Down)
**File:** `server/src/core/MaxHeap.ts`

**What is it?**
Algorithms to maintain the heap property after insertions or deletions.

**Sift Up (after insert):**
1. Add element at the bottom
2. Compare with parent
3. If larger, swap with parent
4. Repeat until heap property restored

**Sift Down (after extract):**
1. Move last element to root
2. Compare with children
3. Swap with larger child if necessary
4. Repeat until heap property restored

**Time Complexity:** O(log n) for both operations

---

### 2.3 Collusion Detection Algorithm
**File:** `server/src/core/AntiAbuse.ts`

**What is it?**
Detects users who might be working together to manipulate votes.

**How it works:**
1. For each pair of users, calculate vote overlap
2. `Similarity = |Common Votes| / |Total Votes by Either|`
3. If similarity > 80%, flag as potential collusion

**Time Complexity:** O(n² × m) where n = users, m = average votes per user

---

### 2.4 TF-IDF (Term Frequency - Inverse Document Frequency)
**File:** `server/src/core/Analytics.ts`

**What is it?**
A numerical statistic reflecting how important a word is to a document.

**Why used?**
Find similar proposals based on text content.

**Formula:**
```
TF(word) = (Times word appears in doc) / (Total words in doc)
IDF(word) = log(Total docs / Docs containing word)
TF-IDF = TF × IDF
```

**Time Complexity:** O(n × m) where n = documents, m = words per document

---

### 2.5 Cosine Similarity
**File:** `server/src/core/Analytics.ts`

**What is it?**
Measures similarity between two vectors (proposal text representations).

**Formula:**
```
similarity = (A · B) / (||A|| × ||B||)
```
Result ranges from 0 (completely different) to 1 (identical).

**Time Complexity:** O(m) where m = vector dimensions

---

### 2.6 Rate Limiting Algorithm (Token Bucket variant)
**File:** `server/src/core/AdvancedAntiBot.ts`

**What is it?**
Limits the number of requests per time window to prevent abuse.

**How it works:**
1. Track request count per IP in a time window (60 seconds)
2. If count > limit (200), block the IP
3. Reset count when window expires

**Time Complexity:** O(1) per request

---

## 3. MACHINE LEARNING MODELS

### 3.1 Naive Bayes Classifier
**File:** `server/src/core/IntelligenceModels.ts`

**What is it?**
A probabilistic classifier based on Bayes' theorem with "naive" independence assumptions.

**Why used?**
Classify proposals into priority levels (high/medium/low) based on text and vote counts.

**How it works:**
1. **Training:** Calculate probability of each word appearing in each class
2. **Prediction:** Multiply probabilities for all words in new proposal
3. **Class:** Choose class with highest probability

**Formula:**
```
P(class|text) = P(class) × ∏ P(word|class)
```

**Time Complexity:**
- Training: O(n × m) where n = documents, m = words per document
- Prediction: O(m) per document

**Confidence Calculation:** Based on probability ratios between classes

---

### 3.2 Random Forest Classifier
**File:** `server/src/core/IntelligenceModels.ts`

**What is it?**
An ensemble of decision trees that votes on the final classification.

**Why used?**
More robust predictions by combining multiple decision trees.

**How it works:**
1. Create multiple decision trees (3 in our implementation)
2. Each tree uses vote count thresholds
3. Final prediction = majority vote of all trees

**Decision Tree Logic:**
```
if (voteCount >= 30) return "high_priority"
else if (voteCount >= 15) return "medium_priority"
else return "low_priority"
```

**Time Complexity:**
- Training: O(n × t) where t = number of trees
- Prediction: O(t) per document

---

### 3.3 Ensemble Classifier
**File:** `server/src/core/IntelligenceModels.ts`

**What is it?**
Combines predictions from multiple models (Naive Bayes + Random Forest).

**Why used?**
More accurate and reliable predictions than any single model.

**How it works:**
1. Get prediction from Naive Bayes
2. Get prediction from Random Forest
3. Simple voting: if both agree, high confidence; if disagree, lower confidence

**Time Complexity:**
- Training: O(n × m) (sum of individual models)
- Prediction: O(m) per document

---

## 4. SECURITY FEATURES

### 4.1 JWT (JSON Web Tokens)
**File:** `server/src/routes/api.ts`

**What is it?**
A compact, URL-safe token for securely transmitting information.

**Why used?**
Stateless authentication - no need to store sessions on server.

**Structure:**
```
Header.Payload.Signature
eyJhbGc...eyJ1c2Vy...SflKxw...
```

**Time Complexity:** O(1) to verify

---

### 4.2 Math CAPTCHA
**File:** `server/src/core/AdvancedAntiBot.ts`

**What is it?**
Human verification using simple math problems (e.g., "What is 23 + 45?")

**Why used?**
Prevent bots from mass-registering or automated voting.

---

### 4.3 Vote Velocity Detection
**File:** `server/src/core/AdvancedAntiBot.ts`

**What is it?**
Detects abnormally fast voting patterns.

**Rules:**
- > 10 votes per minute = suspicious
- > 50 votes per hour = suspicious
- Very regular intervals (low variance) = bot-like

**Time Complexity:** O(v) where v = votes in last hour

---

## 5. SUMMARY TABLE

| Component | Data Structure/Algorithm | Time Complexity |
|-----------|-------------------------|-----------------|
| Rankings | Max Heap | O(log n) insert/update |
| User Storage | Hash Map | O(1) lookup |
| Audit Trail | Linked Hashes (SHA-256) | O(n) verify |
| Password Security | SHA-256 | O(1) hash |
| Similar Proposals | TF-IDF + Cosine Similarity | O(n × m) |
| Collusion Detection | Pairwise Comparison | O(n² × m) |
| Text Classification | Naive Bayes | O(m) predict |
| Priority Prediction | Random Forest | O(t) predict |
| Final Classification | Ensemble (NB + RF) | O(m) predict |
| Rate Limiting | Token Bucket | O(1) per request |
| Authentication | JWT | O(1) verify |

---

## 6. SPACE COMPLEXITY

| Component | Space Complexity | Notes |
|-----------|-----------------|-------|
| Max Heap | O(n) | n = number of proposals |
| User Map | O(u) | u = number of users |
| Proposal Map | O(p) | p = number of proposals |
| Audit Log | O(l) | l = number of log entries |
| Rate Limit Map | O(i) | i = unique IPs |
| Word Probabilities (NB) | O(v × c) | v = vocabulary, c = classes |

---

*Generated for CrowdDecision - Live Voting Platform*
*https://crowddecision-web.onrender.com*
