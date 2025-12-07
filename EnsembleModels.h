#ifndef ENSEMBLE_MODELS_H
#define ENSEMBLE_MODELS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <memory>
#include <cmath>
#include <algorithm>
#include <numeric>

// Forward declarations
class Proposal;

/**
 * Classification result with confidence scores
 */
struct ClassificationResult {
    std::string label;                              // Predicted class label
    double confidence;                               // Confidence score (0-1)
    std::unordered_map<std::string, double> classProbabilities;  // All class probabilities
    std::vector<std::string> topFeatures;           // Top contributing features
    std::string modelName;                          // Name of model that made prediction
    
    ClassificationResult() : confidence(0.0) {}
    ClassificationResult(const std::string& lbl, double conf, const std::string& model = "")
        : label(lbl), confidence(conf), modelName(model) {}
};

/**
 * Ensemble prediction combining multiple models
 */
struct EnsemblePrediction {
    std::string finalLabel;                         // Final predicted label
    double confidence;                               // Overall confidence
    std::unordered_map<std::string, double> modelVotes;  // Per-model predictions
    std::unordered_map<std::string, double> weightedVotes;  // Weighted votes per class
    std::string explanation;                        // How decision was made
    
    EnsemblePrediction() : confidence(0.0) {}
};

/**
 * Feature vector for ML models
 */
struct FeatureVector {
    std::string instanceId;                         // Proposal ID or instance identifier
    std::unordered_map<std::string, double> features;  // Feature name -> value
    std::vector<std::string> textTokens;            // Tokenized text for NLP
    std::string groundTruthLabel;                   // True label (for training)
    
    FeatureVector() {}
    FeatureVector(const std::string& id) : instanceId(id) {}
};

/**
 * Multinomial Naive Bayes Classifier
 * 
 * Fast text baseline classifier using Bayes' theorem:
 * P(class|features) ∝ P(class) × ∏ P(feature|class)
 */
class NaiveBayesClassifier {
private:
    // P(class) - prior probability of each class
    std::unordered_map<std::string, double> classPriors;
    
    // P(feature|class) - likelihood of feature given class
    std::unordered_map<std::string, std::unordered_map<std::string, double>> featureLikelihoods;
    
    // Vocabulary and class statistics
    std::unordered_set<std::string> vocabulary;
    std::unordered_map<std::string, int> classDocCounts;
    std::unordered_map<std::string, std::unordered_map<std::string, int>> featureCounts;
    
    int totalDocuments;
    double smoothingAlpha;  // Laplace smoothing parameter
    
    bool isTrained;
    
    // Helper methods
    double calculateLogProbability(const std::vector<std::string>& tokens,
                                   const std::string& className) const;
    
public:
    /**
     * Constructor
     * @param alpha Laplace smoothing parameter (default: 1.0)
     */
    NaiveBayesClassifier(double alpha = 1.0);
    
    /**
     * Train the classifier
     * @param features Vector of feature vectors with labels
     */
    void train(const std::vector<FeatureVector>& features);
    
    /**
     * Predict class for a feature vector
     * @param features Feature vector to classify
     * @return Classification result with probabilities
     */
    ClassificationResult predict(const FeatureVector& features) const;
    
    /**
     * Get class probabilities for a feature vector
     * @param features Feature vector to classify
     * @return Map of class -> probability
     */
    std::unordered_map<std::string, double> predictProbabilities(
        const FeatureVector& features) const;
    
    /**
     * Check if model is trained
     */
    bool isModelTrained() const { return isTrained; }
    
    /**
     * Get model statistics
     */
    std::string getModelInfo() const;
};

/**
 * Decision Tree Node (for Random Forest)
 */
struct DecisionTreeNode {
    bool isLeaf;
    std::string featureName;        // Feature to split on
    double threshold;                // Split threshold
    std::string label;              // Label if leaf node
    double confidence;              // Confidence at leaf
    
    std::shared_ptr<DecisionTreeNode> leftChild;   // <= threshold
    std::shared_ptr<DecisionTreeNode> rightChild;  // > threshold
    
    DecisionTreeNode() : isLeaf(false), threshold(0.0), confidence(0.0) {}
};

/**
 * Decision Tree Classifier
 * 
 * Binary decision tree for classification
 */
class DecisionTree {
private:
    std::shared_ptr<DecisionTreeNode> root;
    int maxDepth;
    int minSamplesSplit;
    int currentDepth;
    
    // Helper methods
    std::shared_ptr<DecisionTreeNode> buildTree(
        const std::vector<FeatureVector>& samples,
        const std::vector<std::string>& featureNames,
        int depth);
    
    double calculateGini(const std::vector<std::string>& labels) const;
    
    std::pair<std::string, double> findBestSplit(
        const std::vector<FeatureVector>& samples,
        const std::vector<std::string>& featureNames) const;
    
    std::string getMajorityClass(const std::vector<FeatureVector>& samples) const;
    
    ClassificationResult traverse(const FeatureVector& features,
                                 std::shared_ptr<DecisionTreeNode> node) const;
    
public:
    /**
     * Constructor
     * @param maxDepth Maximum tree depth (default: 10)
     * @param minSamples Minimum samples to split (default: 2)
     */
    DecisionTree(int maxDepth = 10, int minSamples = 2);
    
    /**
     * Train the decision tree
     * @param features Training feature vectors
     * @param featureNames Names of features to use
     */
    void train(const std::vector<FeatureVector>& features,
              const std::vector<std::string>& featureNames);
    
    /**
     * Predict class for a feature vector
     * @param features Feature vector to classify
     * @return Classification result
     */
    ClassificationResult predict(const FeatureVector& features) const;
    
    /**
     * Get tree depth
     */
    int getDepth() const { return currentDepth; }
};

/**
 * Random Forest Classifier
 * 
 * Ensemble of decision trees with bootstrap aggregating (bagging)
 * Combines predictions from multiple trees via majority voting
 */
class RandomForestClassifier {
private:
    std::vector<DecisionTree> trees;
    int numTrees;
    int maxDepth;
    int minSamplesSplit;
    double featureSamplingRatio;  // Fraction of features to consider per tree
    
    bool isTrained;
    std::vector<std::string> featureNames;
    
    // Helper: Create bootstrap sample
    std::vector<FeatureVector> createBootstrapSample(
        const std::vector<FeatureVector>& features) const;
    
    // Helper: Sample features for tree
    std::vector<std::string> sampleFeatures(
        const std::vector<std::string>& allFeatures) const;
    
public:
    /**
     * Constructor
     * @param numTrees Number of trees in forest (default: 10)
     * @param maxDepth Maximum depth per tree (default: 10)
     * @param minSamples Minimum samples to split (default: 2)
     * @param featureRatio Fraction of features per tree (default: 0.7)
     */
    RandomForestClassifier(int numTrees = 10, int maxDepth = 10,
                          int minSamples = 2, double featureRatio = 0.7);
    
    /**
     * Train the random forest
     * @param features Training feature vectors with labels
     */
    void train(const std::vector<FeatureVector>& features);
    
    /**
     * Predict class for a feature vector
     * @param features Feature vector to classify
     * @return Classification result with aggregated votes
     */
    ClassificationResult predict(const FeatureVector& features) const;
    
    /**
     * Get class probabilities (based on tree votes)
     * @param features Feature vector to classify
     * @return Map of class -> probability
     */
    std::unordered_map<std::string, double> predictProbabilities(
        const FeatureVector& features) const;
    
    /**
     * Check if model is trained
     */
    bool isModelTrained() const { return isTrained; }
    
    /**
     * Get model statistics
     */
    std::string getModelInfo() const;
};

/**
 * Meta Learner for Model Stacking
 * 
 * Combines predictions from multiple base models using logistic regression
 */
class MetaLearner {
private:
    std::unordered_map<std::string, double> weights;  // Model name -> weight
    std::vector<std::string> modelNames;
    bool isTrained;
    
public:
    MetaLearner();
    
    /**
     * Train meta learner on base model predictions
     * @param basePredictions Map of model name -> predictions for each instance
     * @param groundTruth True labels
     */
    void train(const std::unordered_map<std::string, std::vector<ClassificationResult>>& basePredictions,
              const std::vector<std::string>& groundTruth);
    
    /**
     * Predict using weighted combination of base models
     * @param basePredictions Map of model name -> prediction
     * @return Final ensemble prediction
     */
    EnsemblePrediction predict(
        const std::unordered_map<std::string, ClassificationResult>& basePredictions) const;
    
    /**
     * Get model weights
     */
    std::unordered_map<std::string, double> getWeights() const { return weights; }
    
    bool isModelTrained() const { return isTrained; }
};

/**
 * Ensemble Model Combiner
 * 
 * Combines multiple ML models (Naive Bayes, Random Forest, etc.) using:
 * 1. Simple voting
 * 2. Weighted voting
 * 3. Meta-learning (stacking)
 */
class EnsembleClassifier {
private:
    NaiveBayesClassifier naiveBayes;
    RandomForestClassifier randomForest;
    MetaLearner metaLearner;
    
    std::string ensembleStrategy;  // "voting", "weighted", "stacking"
    bool useNaiveBayes;
    bool useRandomForest;
    
    bool isTrained;
    
public:
    /**
     * Constructor
     * @param strategy Ensemble strategy: "voting", "weighted", or "stacking"
     */
    EnsembleClassifier(const std::string& strategy = "weighted");
    
    /**
     * Configure which models to use
     * @param useNB Use Naive Bayes
     * @param useRF Use Random Forest
     */
    void configureModels(bool useNB, bool useRF);
    
    /**
     * Train all base models and meta learner
     * @param features Training feature vectors with labels
     */
    void train(const std::vector<FeatureVector>& features);
    
    /**
     * Predict using ensemble
     * @param features Feature vector to classify
     * @return Ensemble prediction with explanation
     */
    EnsemblePrediction predict(const FeatureVector& features) const;
    
    /**
     * Get predictions from individual models
     * @param features Feature vector to classify
     * @return Map of model name -> prediction
     */
    std::unordered_map<std::string, ClassificationResult> getIndividualPredictions(
        const FeatureVector& features) const;
    
    /**
     * Evaluate ensemble on test set
     * @param testFeatures Test feature vectors with ground truth labels
     * @return Accuracy score
     */
    double evaluate(const std::vector<FeatureVector>& testFeatures) const;
    
    /**
     * Get confusion matrix
     * @param testFeatures Test feature vectors
     * @return Map of (true_label, predicted_label) -> count
     */
    std::map<std::pair<std::string, std::string>, int> getConfusionMatrix(
        const std::vector<FeatureVector>& testFeatures) const;
    
    /**
     * Get ensemble statistics
     */
    std::string getEnsembleInfo() const;
    
    bool isModelTrained() const { return isTrained; }
};

/**
 * Feature Extractor for Proposals
 * 
 * Extracts features from proposals for ML models
 */
class ProposalFeatureExtractor {
public:
    /**
     * Extract features from a proposal
     * @param proposal Proposal to extract features from
     * @param includeText Include text tokens for NLP models
     * @return Feature vector
     */
    static FeatureVector extractFeatures(const std::shared_ptr<Proposal>& proposal,
                                        bool includeText = true);
    
    /**
     * Extract features from multiple proposals
     * @param proposals Vector of proposals
     * @param labels Ground truth labels (optional)
     * @return Vector of feature vectors
     */
    static std::vector<FeatureVector> extractBatch(
        const std::vector<std::shared_ptr<Proposal>>& proposals,
        const std::vector<std::string>& labels = {});
    
    /**
     * Get feature names
     * @return Vector of feature names
     */
    static std::vector<std::string> getFeatureNames();
};

#endif // ENSEMBLE_MODELS_H
