# CrowdDecision — Final Documentation for AI Coding Agents

**Project**: Crowd-sourced Decision-Making Platform (CrowdDecision)  
**Target consumers**: AI coding agents (e.g., Windsurf), developer teams, reviewers  
**Version**: 1.0  
**Date**: 2025-11-09

## 1. Executive Summary

CrowdDecision is a production-ready architecture and implementation for a crowd-sourced decision-making platform that:
- Ingests user-submitted proposals and community votes
- Detects malicious voting behavior (bots, collusion)
- Ranks decisions by weighted relevance to configurable core topics and subtopics
- Outputs classification labels and ranked recommendations using modern ML/ensemble models

The solution combines:
- **Classical DSA**: Hashmaps, priority queues, binary search over sorted vectors
- **NLP**: Cosine similarity, Jaccard index, TF-IDF, word/sentence embeddings
- **Statistical normalization**: Min-max, z-score
- **ML**: Logistic regression, Naive Bayes, Random Forest, Gradient Boosted Trees
- **Anti-abuse**: Bot detection, collusion detection, user credibility scoring

## 2. Goals & Requirements

### Functional Requirements
- ✅ Ingest proposals and votes in real-time
- ✅ Break core topics into configurable subtopics and compute relevance
- ✅ Rank decisions by weighted relevance and confidence (rank + percentile)
- ✅ Support text similarity & clustering (cosine + Jaccard + TF-IDF)
- ✅ Detect bots and collusion using velocity analysis and graph-based coordination detection
- ✅ Assign per-user credibility/trust scores and use in vote weighting
- ✅ Provide classification labels using ML models
- ✅ Real-time threat assessment and alerts

### Non-functional Requirements
- **Low-latency**: Sub-second ranking for interactive UI
- **Scalable**: Support thousands of concurrent users & votes/second
- **Explainable**: Rank features and label reasons available for auditing
- **Secure**: Resilient to ballot stuffing, Sybil attacks

## 3. High-Level Architecture

### Components
1. **Ingest API** (REST/gRPC/WebSocket) — accepts proposals, votes, comments, user metadata
2. **Stream Processor** (Kafka/Kinesis) — buffer and stream events for real-time processing
3. **Preprocessing & NLP Service** — text cleaning, tokenization, TF-IDF, embeddings, Jaccard sets
4. **Similarity & Ranking Engine** — computes similarity matrices, weighted relevance scores
5. **Anti-Abuse Engine** — bot/velocity analysis, collusion detection, user credibility scoring
6. **ML Classification Service** — training and inference for labels (Logistic Regression, Random Forest, XGBoost)
7. **Database** — Postgres/CockroachDB + vector DB (Faiss/Milvus/Pinecone) + time-series DB
8. **Search & Index** — inverted index (Elasticsearch/OpenSearch) for fast term search
9. **API Gateway & Frontend** — query endpoints for ranked decisions, admin dashboard
10. **Monitoring & Alerting** — Prometheus + Grafana + real-time alerts

### Dataflow
```
User → Ingest API → Stream Processor → (Preprocessing → Vector DB) + 
(Anti-Abuse Engine → User Trust DB) → Ranking Engine → ML Classifier → 
Output (ranked list + label + explanations) → Frontend
```

## 4. Data Model and Key Data Structures

### Entities
```cpp
User {
    string user_id
    timestamp created_at
    map metadata
    double trust_score
    vector<Vote> behavior_history
    double consistency_score
}

Proposal {
    string proposal_id
    string user_id
    string text
    map metadata
    timestamp timestamp
    string embedding_id
    vector<double> embedding_vector
}

Vote {
    string vote_id
    string user_id
    string proposal_id
    double weight
    timestamp timestamp
    string ip_hash
    string device_hash
}

SubTopic {
    string id
    string name
    vector<string> keywords
    double weight
}

DecisionRank {
    string proposal_id
    double relevance_score
    double percentile
    int rank
    string label
    string explanation
    double consistency_bonus
}
```

### In-memory / Algorithmic Data Structures
- **HashMap**: `proposal_id → proposal`, `user_id → trust_score`, `keyword → posting list`
- **Vector**: Dense embedding vectors per proposal and per sub-topic prototype
- **Sorted Vector**: Proposals sorted by relevance (binary search for threshold detection)
- **Priority Queue**: Min-heap of size N to maintain top N proposals
- **Similarity Matrix**: MxM matrix with pairwise cosine similarity (sparse CSR if large)
- **Graph**: Adjacency lists for collusion detection (user co-voting patterns)

## 5. NLP and Similarity Computations

### Preprocessing Steps
1. Lowercase, unicode normalization
2. Remove punctuation (configurable), stop words (optional)
3. Lemmatization
4. Extract tokens and shingles (n-grams for Jaccard)

### Feature Representations
- **Bag-of-words / TF-IDF**: Traditional models and Jaccard indexing
- **Dense embeddings**: sentence-transformers (SBERT) for cosine similarity
- **Keyword vectors**: Per sub-topic weighted keyword vector (TF-IDF or embedding centroid)

### Similarity Metrics
- **Cosine similarity**: On dense or TF-IDF vectors for semantic similarity
- **Jaccard index**: On sets of shingles or keyword sets for surface-level overlap
- **Hybrid score**: `hybrid = α*cosine + β*jaccard + γ*tfidf_score` where `α+β+γ=1`

### Normalization
- **Min-Max**: Normalize to [0,1]
- **Z-score**: Standardize to mean=0, std=1

### Similarity Matrix Use-Cases
- Duplicate detection (high cosine & high Jaccard)
- Cluster proposals into groups (Agglomerative/DBSCAN on similarity distances)
- Identify near-duplicates to de-bias votes

## 6. Subtopic Decomposition & Weighted Relevance

### Subtopic Definition
Each core topic decomposes to configurable subtopics:
```cpp
SubTopic {
    vector<string> keywords  // manual / auto-expanded
    double weight            // importance relative to core topic
}
```

### Relevance Scoring
For proposal P and core topic T with subtopics S_i:
1. Compute `sim_i = hybrid_similarity(P, prototype(S_i))` per subtopic
2. Compute `weighted_relevance = Σ_i (w_i * sim_i)` where w_i is subtopic weight
3. Normalize via min-max across batch or historical distribution
4. Compute percentile for human-friendly interpretation

### Ranking
Maintain priority queue of top proposals by:
```
final_score = weighted_relevance × trust_adjustment × recency_factor × consistency_bonus
```

## 7. Weight-Based Consistency and Stability Scoring

### Concept
Each user has two similarity signals:
- **Local proximity**: How close a single proposal is to core/subtopics
- **Global consistency**: How stable that user's proposals are across all subtopics over time

### Mathematical Formulation
```
sim_i_j = similarity of user i's j-th proposal to core topic
μ_i = mean(sim_i_j) across all j
σ_i = std_dev(sim_i_j)
consistency_score_i = 1 / (1 + σ_i)  // bounded in (0,1]
```

### Final Weighted Score
```
final_weighted_score(D) = α × relevance_score(D)
                        + β × consistency_score_i
                        + γ × trust_score_i
                        + δ × recency_factor

Typical config: α=0.55, β=0.25, γ=0.15, δ=0.05
```

### Implementation Notes
- Maintain rolling window of each user's last n proposals with similarity scores
- Compute σ_i periodically (hourly or per topic update)
- Store `consistency_score_i` in user profile table or cache
- New users start with `consistency_score = 0.5` (neutral)
- Erratic behavior (high variance) → 0.1-0.3 consistency
- Stable users (low variance) → 0.9-1.0 consistency

## 8. Time-Based Filtering and Priority

### Time-Based Rules
- **Recency decay**: Multiply score by `exp(-λ × age_hours)`
- **Time windows**: Sliding windows (last 24h, 7 days) for situational ranking

### Priority Overrides
- Admin flagged proposals override with `admin_priority` boost
- Emergency topics temporarily increase subtopic weights

## 9. Anti-Abuse: Bot & Collusion Detection, User Credibility

### Bot Detection (Velocity Analysis)
- Track per-user voting rates over sliding windows (per minute/hour/day)
- Heuristics: `>X votes/min` or `<Y ms avg inter-vote gap` → suspicious
- Combine with device/IP clustering
- Output `bot_likelihood` score (0-1) from classifier

### Collusion Detection
- Build co-voting graph: nodes=users, edge weight=co-votes within short window
- Use community detection (Louvain) or thresholded clique detection
- Compute `collusion_score` per user and per group

### User Credibility Scoring
Features:
- Account age
- Device diversity
- Historical agreement with majority
- Account verification status
- Bot-likelihood
- Collusion-score
- Vote consistency
- Report/flag history

Output: `trust_score` in [0,1] via weighted logistic regression or ensemble

Use: `effective_vote_weight = raw_vote_weight × trust_score`

### Real-Time Threat Assessment
- Stream alerts when group's `collusion_score` or `bot_likelihood` exceeds thresholds
- Admin UI with graph visualizations and evidence

## 10. Machine Learning Models

### Baseline Models
- **Logistic Regression**: Quick, interpretable baseline
- **Multinomial Naive Bayes**: Fast text baseline

### Advanced Models
- **Random Forest / XGBoost / LightGBM**: Tabular + text features ensemble
- **Neural models**: Small feedforward nets or fine-tuned transformer encoder

### Ensemble Strategy
Stack predictions from:
1. Text-only model (Naive Bayes or transformer)
2. Metadata-only model (XGBoost)
3. Meta-learner (logistic regression) → final label and confidence

### Feature Engineering
**Text**: TF-IDF, embedding centroids, topic-similarity, Jaccard with subtopic keywords, duplicate-flag  
**Metadata**: trust_score, vote_count, vote_velocity, avg_vote_time_delta, age_hours, flagged_count

### Training Strategy
- Train on historical labeled examples with time-aware CV
- Use class-weighting or focal loss for imbalanced labels

### Evaluation Metrics
- **Classification**: Precision, recall, F1 per class, ROC-AUC
- **Ranking**: NDCG@k, MAP, Precision@k
- **Anti-abuse**: Detection-rate vs false-positive rate curve

## 11. Implementation Status

### ✅ Already Implemented (C++)
- User registration & management with unique IDs
- Proposal creation and voting
- Dynamic rankings using max-heap
- Tamper-evident logging with hash chaining
- AI-powered recommendations
- Sentiment analysis with quality scoring
- Fraud detection with anomaly detection
- Predictive analytics
- Smart notifications
- MinMax normalization & Z-score standardization
- Jaccard index & Cosine similarity
- Topic analysis & decomposition with binary search
- Logistic regression classification
- Time-based filtering with priority queues
- Decision ranking engine with weighted relevance
- Similarity matrix construction
- Rank & percentile system

### 🔨 New Implementations Required

#### 1. Consistency & Stability Scoring
- `ConsistencyScorer` class to track user proposal history
- Rolling window calculation of mean and std dev per user
- Integration with ranking formula

#### 2. Enhanced Anti-Abuse Engine
- Velocity tracking per user (votes/min, votes/hour)
- Device fingerprinting and IP hash tracking
- Co-voting graph construction
- Community detection for collusion
- Streaming alerts for suspicious behavior

#### 3. User Credibility System
- Multi-factor credibility calculation
- Trust score weighting in vote calculations
- Historical behavior analysis

#### 4. Ensemble ML Models
- Naive Bayes classifier for text
- Random Forest simulation (multiple decision trees)
- Model stacking and meta-learning

#### 5. Embedding Support
- Integration with sentence-transformers (simulated)
- Vector storage and similarity search
- Embedding-based semantic search

## 12. Algorithms & Pseudocode

### Preprocessing & Embedding
```cpp
function preprocess(text):
    text = normalize_unicode(lowercase(text))
    tokens = tokenize(text)
    tokens = lemmatize(tokens)
    shingles = generate_shingles(tokens, n=3)
    return {tokens, shingles, cleaned_text}

function compute_embedding(cleaned_text):
    return sentence_transformer.encode(cleaned_text)
```

### Weighted Relevance & Ranking (Streaming)
```cpp
heap = min_heap(size=K)
for each proposal in incoming_stream:
    features = preprocess_and_features(proposal)
    sim_vector = [hybrid_similarity(proposal, subtopic_proto) 
                  for subtopic_proto in subtopics]
    weighted_relevance = dot_product(sim_vector, subtopic_weights)
    normalized_score = normalize(weighted_relevance)
    trust_adj = user_trust_score(proposal.user_id)
    consistency_adj = user_consistency_score(proposal.user_id)
    recency = exp(-lambda * age_hours(proposal))
    final_score = 0.55*normalized_score + 0.25*consistency_adj + 
                  0.15*trust_adj + 0.05*recency
    if heap.size < K or final_score > heap.peek().score:
        heap.push((final_score, proposal_id))
        if heap.size > K: heap.pop()
```

### Bot Velocity Detection (Sliding Window)
```cpp
for each vote_event(user_id, timestamp):
    window = sliding_window_for(user_id, last_T_seconds)
    rate = window.count / T
    mean_delta = average_inter_vote_gap(window)
    if rate > RATE_THRESHOLD or mean_delta < DELTA_THRESHOLD:
        mark_user_suspicious(user_id)
        increase_bot_likelihood(user_id)
```

### Collusion Detection (Batch Graph)
```cpp
// Build co-vote adjacency
for each time_window W:
    for each proposal in proposals_in(W):
        voters = set(voters_of(proposal))
        for each pair (u,v) in combinations(voters,2):
            adjacency[u][v] += 1
// Normalize by expected co-vote frequency
compute_communities(adjacency)  // Louvain or community detection
flag communities with high internal co-vote rate
```

## 13. Configuration & Hyperparameters

```cpp
// Embedding model
embedding_model = "all-MiniLM-L6-v2"  // lightweight yet strong

// Hybrid similarity weights
alpha_cosine = 0.6
beta_jaccard = 0.3
gamma_tfidf = 0.1

// Consistency scoring weights
alpha_relevance = 0.55
beta_consistency = 0.25
gamma_trust = 0.15
delta_recency = 0.05

// Time decay
lambda_hours = 0.05

// Bot detection thresholds
RATE_THRESHOLD = 30  // votes per minute
DELTA_THRESHOLD_MS = 200  // average inter-vote gap in ms

// Priority queue size
TOP_K = 50
```

## 14. API Endpoints (Conceptual)

```
POST /api/proposals
  body: { user_id, text, metadata }

POST /api/votes
  body: { user_id, proposal_id, vote_type, weight? }

GET /api/proposals/top?topic=<>&k=20
  response: list of {proposal_id, score, percentile, label, explanation}

GET /api/proposals/:id
  response: proposal with similarity to subtopics and history

GET /api/admin/threats
  response: suspicious users/communities feed

GET /api/users/:id/consistency
  response: { consistency_score, variance, mean_similarity }
```

## 15. Testing & Evaluation

### Unit & Integration Tests
- Unit test each preprocessing function and similarity computation
- Integration tests for streaming pipeline (mock producer/consumer)

### Offline Evaluation
- Holdout test set with labeled proposals and known attack scenarios
- Evaluate ranking metrics (NDCG@10), classification metrics (F1), anti-abuse detection

### Online Evaluation
- A/B testing with different trust_score weightings and subtopic weightings
- Measure engagement and quality ratings by human moderators

## 16. Security & Privacy

- Salt + hash IP and device fingerprints
- Keep raw IPs only in ephemeral logs with strict retention
- Rate-limit ingest endpoints
- Require API keys for high-volume producers
- Anomaly detection to auto-block suspicious IP ranges

## 17. Deployment Notes

- Deploy stream processing on Kubernetes with autoscaling consumers
- Use horizontal partitioning for similarity computations
- For large-scale pairwise similarity, use ANN indexing (Faiss)
- Cache top-K per topic in Redis; invalidate on new high-scoring proposals

## 18. Operational Playbook

### Incident Responses
- **Bot spike**: Auto-throttle IPs, increase verification (captcha), flag users for manual review
- **Collusion cluster found**: Quarantine associated votes, notify admin, shadow-ban accounts
- **Model drift observed**: Retrain weekly with latest labeled data; schedule nightly validation runs

## 19. Project Structure

```
ds cp 3/
├── CROWDDECISION_ARCHITECTURE.md       (this file)
├── README.md                            (existing)
├── IMPLEMENTATION_SUMMARY.md            (existing)
├── ADVANCED_FEATURES.md                 (existing)
├── VotingSystem.h                       (existing)
├── VotingSystem.cpp                     (existing)
├── IntelligenceEngine.h                 (existing)
├── IntelligenceEngine.cpp               (existing)
├── AdvancedAnalytics.h                  (existing)
├── AdvancedAnalytics.cpp                (existing)
├── AdvancedAnalytics_Part2.cpp          (existing)
├── AdvancedAnalytics_Part3.cpp          (existing)
├── ConsistencyScorer.h                  (new - to be created)
├── ConsistencyScorer.cpp                (new - to be created)
├── AntiAbuseEngine.h                    (new - to be created)
├── AntiAbuseEngine.cpp                  (new - to be created)
├── EnsembleModels.h                     (new - to be created)
├── EnsembleModels.cpp                   (new - to be created)
├── StreamProcessor.h                    (new - conceptual stub)
├── StreamProcessor.cpp                  (new - conceptual stub)
└── Makefile                             (update)
```

## 20. Next Steps

1. ✅ Create this comprehensive documentation
2. 🔨 Implement `ConsistencyScorer` class
3. 🔨 Enhance `AntiAbuseEngine` with velocity tracking and collusion detection
4. 🔨 Add `UserCredibilityScorer` 
5. 🔨 Implement `EnsembleModels` (Naive Bayes, Random Forest simulation)
6. 🔨 Add embedding support stubs
7. 🔨 Create streaming architecture conceptual stubs
8. 🔨 Update integration demos

## 21. Conclusion

This document serves as the complete architectural specification for CrowdDecision, a production-ready crowd-sourced decision-making platform. The implementation balances classical DSA, modern ML/NLP techniques, and robust anti-abuse mechanisms to deliver an explainable, secure, and scalable solution.

The existing C++ codebase already implements the majority of core features. The remaining implementations focus on:
- Consistency & stability scoring
- Enhanced anti-abuse with velocity and collusion detection
- Ensemble ML models
- Streaming architecture stubs

All new code will maintain the same quality standards: modular design, comprehensive documentation, thorough testing, and production-ready implementation.

---

**Status**: Architecture documentation complete. Implementation in progress.
