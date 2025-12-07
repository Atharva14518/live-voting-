#include "EnsembleModels.h"
#include "VotingSystem.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <set>

// ==================== Naive Bayes Classifier ====================

NaiveBayesClassifier::NaiveBayesClassifier(double alpha)
    : smoothingAlpha(alpha), totalDocuments(0), isTrained(false) {
}

void NaiveBayesClassifier::train(const std::vector<FeatureVector>& features) {
    // Reset state
    classPriors.clear();
    featureLikelihoods.clear();
    vocabulary.clear();
    classDocCounts.clear();
    featureCounts.clear();
    totalDocuments = features.size();
    
    // Count documents per class and build vocabulary
    for (const auto& fv : features) {
        classDocCounts[fv.groundTruthLabel]++;
        for (const auto& token : fv.textTokens) {
            vocabulary.insert(token);
            featureCounts[fv.groundTruthLabel][token]++;
        }
    }
    
    // Calculate class priors: P(class) = count(class) / total
    for (const auto& pair : classDocCounts) {
        classPriors[pair.first] = static_cast<double>(pair.second) / totalDocuments;
    }
    
    // Calculate feature likelihoods with Laplace smoothing
    // P(feature|class) = (count(feature, class) + α) / (count(class) + α * |V|)
    int vocabSize = vocabulary.size();
    for (const auto& classPair : classDocCounts) {
        const std::string& className = classPair.first;
        int classTotal = 0;
        
        // Sum all feature counts for this class
        for (const auto& featurePair : featureCounts[className]) {
            classTotal += featurePair.second;
        }
        
        // Calculate likelihood for each feature
        for (const std::string& feature : vocabulary) {
            int featureCount = featureCounts[className][feature];
            double likelihood = (featureCount + smoothingAlpha) / 
                               (classTotal + smoothingAlpha * vocabSize);
            featureLikelihoods[className][feature] = likelihood;
        }
    }
    
    isTrained = true;
}

double NaiveBayesClassifier::calculateLogProbability(
    const std::vector<std::string>& tokens,
    const std::string& className) const {
    
    // Start with log prior
    double logProb = std::log(classPriors.at(className));
    
    // Add log likelihoods for each token
    for (const std::string& token : tokens) {
        if (featureLikelihoods.at(className).count(token)) {
            logProb += std::log(featureLikelihoods.at(className).at(token));
        } else {
            // Unseen feature - use smoothing
            int vocabSize = vocabulary.size();
            int classTotal = 0;
            for (const auto& pair : featureCounts.at(className)) {
                classTotal += pair.second;
            }
            double smoothedProb = smoothingAlpha / (classTotal + smoothingAlpha * vocabSize);
            logProb += std::log(smoothedProb);
        }
    }
    
    return logProb;
}

ClassificationResult NaiveBayesClassifier::predict(const FeatureVector& features) const {
    if (!isTrained) {
        return ClassificationResult("unknown", 0.0, "NaiveBayes");
    }
    
    auto probabilities = predictProbabilities(features);
    
    // Find class with highest probability
    std::string bestClass;
    double maxProb = -std::numeric_limits<double>::infinity();
    for (const auto& pair : probabilities) {
        if (pair.second > maxProb) {
            maxProb = pair.second;
            bestClass = pair.first;
        }
    }
    
    ClassificationResult result(bestClass, maxProb, "NaiveBayes");
    result.classProbabilities = probabilities;
    
    return result;
}

std::unordered_map<std::string, double> NaiveBayesClassifier::predictProbabilities(
    const FeatureVector& features) const {
    
    std::unordered_map<std::string, double> probabilities;
    
    if (!isTrained) {
        return probabilities;
    }
    
    // Calculate log probabilities for each class
    std::unordered_map<std::string, double> logProbs;
    for (const auto& pair : classPriors) {
        const std::string& className = pair.first;
        logProbs[className] = calculateLogProbability(features.textTokens, className);
    }
    
    // Convert log probabilities to probabilities using softmax
    double maxLogProb = -std::numeric_limits<double>::infinity();
    for (const auto& pair : logProbs) {
        maxLogProb = std::max(maxLogProb, pair.second);
    }
    
    double sumExp = 0.0;
    for (const auto& pair : logProbs) {
        sumExp += std::exp(pair.second - maxLogProb);
    }
    
    for (const auto& pair : logProbs) {
        probabilities[pair.first] = std::exp(pair.second - maxLogProb) / sumExp;
    }
    
    return probabilities;
}

std::string NaiveBayesClassifier::getModelInfo() const {
    std::stringstream ss;
    ss << "Naive Bayes Classifier\n";
    ss << "  Trained: " << (isTrained ? "Yes" : "No") << "\n";
    ss << "  Classes: " << classPriors.size() << "\n";
    ss << "  Vocabulary size: " << vocabulary.size() << "\n";
    ss << "  Training documents: " << totalDocuments << "\n";
    ss << "  Smoothing alpha: " << smoothingAlpha << "\n";
    return ss.str();
}

// ==================== Decision Tree ====================

DecisionTree::DecisionTree(int maxDepth, int minSamples)
    : maxDepth(maxDepth), minSamplesSplit(minSamples), currentDepth(0) {
}

double DecisionTree::calculateGini(const std::vector<std::string>& labels) const {
    if (labels.empty()) return 0.0;
    
    std::unordered_map<std::string, int> counts;
    for (const auto& label : labels) {
        counts[label]++;
    }
    
    double gini = 1.0;
    int total = labels.size();
    for (const auto& pair : counts) {
        double prob = static_cast<double>(pair.second) / total;
        gini -= prob * prob;
    }
    
    return gini;
}

std::string DecisionTree::getMajorityClass(const std::vector<FeatureVector>& samples) const {
    std::unordered_map<std::string, int> counts;
    for (const auto& sample : samples) {
        counts[sample.groundTruthLabel]++;
    }
    
    std::string majority;
    int maxCount = 0;
    for (const auto& pair : counts) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
            majority = pair.first;
        }
    }
    
    return majority;
}

std::pair<std::string, double> DecisionTree::findBestSplit(
    const std::vector<FeatureVector>& samples,
    const std::vector<std::string>& featureNames) const {
    
    double bestGiniGain = 0.0;
    std::string bestFeature;
    double bestThreshold = 0.0;
    
    // Calculate current Gini
    std::vector<std::string> currentLabels;
    for (const auto& sample : samples) {
        currentLabels.push_back(sample.groundTruthLabel);
    }
    double currentGini = calculateGini(currentLabels);
    
    // Try each feature
    for (const auto& featureName : featureNames) {
        // Get all values for this feature
        std::vector<double> values;
        for (const auto& sample : samples) {
            auto it = sample.features.find(featureName);
            if (it != sample.features.end()) {
                values.push_back(it->second);
            }
        }
        
        if (values.empty()) continue;
        
        // Try median as threshold
        std::sort(values.begin(), values.end());
        double threshold = values[values.size() / 2];
        
        // Split samples
        std::vector<std::string> leftLabels, rightLabels;
        for (const auto& sample : samples) {
            auto it = sample.features.find(featureName);
            double value = (it != sample.features.end()) ? it->second : 0.0;
            
            if (value <= threshold) {
                leftLabels.push_back(sample.groundTruthLabel);
            } else {
                rightLabels.push_back(sample.groundTruthLabel);
            }
        }
        
        if (leftLabels.empty() || rightLabels.empty()) continue;
        
        // Calculate weighted Gini after split
        double leftGini = calculateGini(leftLabels);
        double rightGini = calculateGini(rightLabels);
        double weightedGini = (leftLabels.size() * leftGini + rightLabels.size() * rightGini) / samples.size();
        
        double giniGain = currentGini - weightedGini;
        
        if (giniGain > bestGiniGain) {
            bestGiniGain = giniGain;
            bestFeature = featureName;
            bestThreshold = threshold;
        }
    }
    
    return {bestFeature, bestThreshold};
}

std::shared_ptr<DecisionTreeNode> DecisionTree::buildTree(
    const std::vector<FeatureVector>& samples,
    const std::vector<std::string>& featureNames,
    int depth) {
    
    auto node = std::make_shared<DecisionTreeNode>();
    
    // Check stopping criteria
    if (depth >= maxDepth || samples.size() < static_cast<size_t>(minSamplesSplit)) {
        node->isLeaf = true;
        node->label = getMajorityClass(samples);
        node->confidence = 1.0;
        currentDepth = std::max(currentDepth, depth);
        return node;
    }
    
    // Check if all samples have same label
    std::string firstLabel = samples[0].groundTruthLabel;
    bool allSame = true;
    for (const auto& sample : samples) {
        if (sample.groundTruthLabel != firstLabel) {
            allSame = false;
            break;
        }
    }
    
    if (allSame) {
        node->isLeaf = true;
        node->label = firstLabel;
        node->confidence = 1.0;
        currentDepth = std::max(currentDepth, depth);
        return node;
    }
    
    // Find best split
    auto [bestFeature, bestThreshold] = findBestSplit(samples, featureNames);
    
    if (bestFeature.empty()) {
        node->isLeaf = true;
        node->label = getMajorityClass(samples);
        node->confidence = 0.5;
        currentDepth = std::max(currentDepth, depth);
        return node;
    }
    
    // Split samples
    std::vector<FeatureVector> leftSamples, rightSamples;
    for (const auto& sample : samples) {
        auto it = sample.features.find(bestFeature);
        double value = (it != sample.features.end()) ? it->second : 0.0;
        
        if (value <= bestThreshold) {
            leftSamples.push_back(sample);
        } else {
            rightSamples.push_back(sample);
        }
    }
    
    // Create internal node
    node->isLeaf = false;
    node->featureName = bestFeature;
    node->threshold = bestThreshold;
    node->leftChild = buildTree(leftSamples, featureNames, depth + 1);
    node->rightChild = buildTree(rightSamples, featureNames, depth + 1);
    
    return node;
}

void DecisionTree::train(const std::vector<FeatureVector>& features,
                        const std::vector<std::string>& featureNames) {
    currentDepth = 0;
    root = buildTree(features, featureNames, 0);
}

ClassificationResult DecisionTree::traverse(const FeatureVector& features,
                                           std::shared_ptr<DecisionTreeNode> node) const {
    if (!node) {
        return ClassificationResult("unknown", 0.0, "DecisionTree");
    }
    
    if (node->isLeaf) {
        return ClassificationResult(node->label, node->confidence, "DecisionTree");
    }
    
    // Get feature value
    auto it = features.features.find(node->featureName);
    double value = (it != features.features.end()) ? it->second : 0.0;
    
    // Traverse left or right
    if (value <= node->threshold) {
        return traverse(features, node->leftChild);
    } else {
        return traverse(features, node->rightChild);
    }
}

ClassificationResult DecisionTree::predict(const FeatureVector& features) const {
    if (!root) {
        return ClassificationResult("unknown", 0.0, "DecisionTree");
    }
    
    return traverse(features, root);
}

// ==================== Random Forest ====================

RandomForestClassifier::RandomForestClassifier(int numTrees, int maxDepth,
                                             int minSamples, double featureRatio)
    : numTrees(numTrees), maxDepth(maxDepth), minSamplesSplit(minSamples),
      featureSamplingRatio(featureRatio), isTrained(false) {
}

std::vector<FeatureVector> RandomForestClassifier::createBootstrapSample(
    const std::vector<FeatureVector>& features) const {
    
    std::vector<FeatureVector> bootstrap;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, features.size() - 1);
    
    for (size_t i = 0; i < features.size(); i++) {
        int idx = dis(gen);
        bootstrap.push_back(features[idx]);
    }
    
    return bootstrap;
}

std::vector<std::string> RandomForestClassifier::sampleFeatures(
    const std::vector<std::string>& allFeatures) const {
    
    int numFeatures = std::max(1, static_cast<int>(allFeatures.size() * featureSamplingRatio));
    
    std::vector<std::string> sampled;
    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::vector<std::string> shuffled = allFeatures;
    std::shuffle(shuffled.begin(), shuffled.end(), gen);
    
    for (int i = 0; i < numFeatures && i < static_cast<int>(shuffled.size()); i++) {
        sampled.push_back(shuffled[i]);
    }
    
    return sampled;
}

void RandomForestClassifier::train(const std::vector<FeatureVector>& features) {
    trees.clear();
    
    // Extract feature names
    if (!features.empty()) {
        for (const auto& pair : features[0].features) {
            featureNames.push_back(pair.first);
        }
    }
    
    // Train each tree on bootstrap sample
    for (int i = 0; i < numTrees; i++) {
        auto bootstrap = createBootstrapSample(features);
        auto sampledFeatures = sampleFeatures(featureNames);
        
        DecisionTree tree(maxDepth, minSamplesSplit);
        tree.train(bootstrap, sampledFeatures);
        trees.push_back(tree);
    }
    
    isTrained = true;
}

ClassificationResult RandomForestClassifier::predict(const FeatureVector& features) const {
    if (!isTrained || trees.empty()) {
        return ClassificationResult("unknown", 0.0, "RandomForest");
    }
    
    auto probabilities = predictProbabilities(features);
    
    // Find class with highest probability
    std::string bestClass;
    double maxProb = 0.0;
    for (const auto& pair : probabilities) {
        if (pair.second > maxProb) {
            maxProb = pair.second;
            bestClass = pair.first;
        }
    }
    
    ClassificationResult result(bestClass, maxProb, "RandomForest");
    result.classProbabilities = probabilities;
    
    return result;
}

std::unordered_map<std::string, double> RandomForestClassifier::predictProbabilities(
    const FeatureVector& features) const {
    
    std::unordered_map<std::string, int> votes;
    
    // Collect votes from all trees
    for (const auto& tree : trees) {
        auto prediction = tree.predict(features);
        votes[prediction.label]++;
    }
    
    // Convert votes to probabilities
    std::unordered_map<std::string, double> probabilities;
    for (const auto& pair : votes) {
        probabilities[pair.first] = static_cast<double>(pair.second) / trees.size();
    }
    
    return probabilities;
}

std::string RandomForestClassifier::getModelInfo() const {
    std::stringstream ss;
    ss << "Random Forest Classifier\n";
    ss << "  Trained: " << (isTrained ? "Yes" : "No") << "\n";
    ss << "  Number of trees: " << numTrees << "\n";
    ss << "  Max depth: " << maxDepth << "\n";
    ss << "  Feature sampling ratio: " << featureSamplingRatio << "\n";
    return ss.str();
}

// ==================== Meta Learner ====================

MetaLearner::MetaLearner() : isTrained(false) {
}

void MetaLearner::train(
    const std::unordered_map<std::string, std::vector<ClassificationResult>>& basePredictions,
    const std::vector<std::string>& groundTruth) {
    
    // Simple weight learning: weight = accuracy of each model
    weights.clear();
    modelNames.clear();
    
    for (const auto& pair : basePredictions) {
        const std::string& modelName = pair.first;
        const auto& predictions = pair.second;
        
        modelNames.push_back(modelName);
        
        // Calculate accuracy
        int correct = 0;
        for (size_t i = 0; i < predictions.size() && i < groundTruth.size(); i++) {
            if (predictions[i].label == groundTruth[i]) {
                correct++;
            }
        }
        
        double accuracy = static_cast<double>(correct) / groundTruth.size();
        weights[modelName] = accuracy;
    }
    
    isTrained = true;
}

EnsemblePrediction MetaLearner::predict(
    const std::unordered_map<std::string, ClassificationResult>& basePredictions) const {
    
    EnsemblePrediction ensemble;
    
    if (!isTrained) {
        ensemble.finalLabel = "unknown";
        ensemble.confidence = 0.0;
        return ensemble;
    }
    
    // Aggregate weighted votes
    std::unordered_map<std::string, double> classScores;
    double totalWeight = 0.0;
    
    for (const auto& pair : basePredictions) {
        const std::string& modelName = pair.first;
        const ClassificationResult& prediction = pair.second;
        
        auto it = weights.find(modelName);
        double weight = (it != weights.end()) ? it->second : 0.5;
        
        ensemble.modelVotes[modelName] = prediction.confidence;
        classScores[prediction.label] += weight * prediction.confidence;
        totalWeight += weight;
    }
    
    // Normalize and find best class
    std::string bestClass;
    double maxScore = 0.0;
    for (const auto& pair : classScores) {
        double normalizedScore = pair.second / totalWeight;
        ensemble.weightedVotes[pair.first] = normalizedScore;
        
        if (normalizedScore > maxScore) {
            maxScore = normalizedScore;
            bestClass = pair.first;
        }
    }
    
    ensemble.finalLabel = bestClass;
    ensemble.confidence = maxScore;
    
    // Generate explanation
    std::stringstream ss;
    ss << "Weighted ensemble: ";
    for (const auto& pair : basePredictions) {
        ss << pair.first << "(" << pair.second.label << ") ";
    }
    ensemble.explanation = ss.str();
    
    return ensemble;
}

// ==================== Ensemble Classifier ====================

EnsembleClassifier::EnsembleClassifier(const std::string& strategy)
    : ensembleStrategy(strategy), useNaiveBayes(true), useRandomForest(true), isTrained(false) {
}

void EnsembleClassifier::configureModels(bool useNB, bool useRF) {
    useNaiveBayes = useNB;
    useRandomForest = useRF;
}

void EnsembleClassifier::train(const std::vector<FeatureVector>& features) {
    // Train Naive Bayes
    if (useNaiveBayes) {
        naiveBayes.train(features);
    }
    
    // Train Random Forest
    if (useRandomForest) {
        randomForest.train(features);
    }
    
    // Train meta learner if using stacking
    if (ensembleStrategy == "stacking") {
        std::unordered_map<std::string, std::vector<ClassificationResult>> basePredictions;
        std::vector<std::string> groundTruth;
        
        for (const auto& fv : features) {
            groundTruth.push_back(fv.groundTruthLabel);
            
            if (useNaiveBayes) {
                basePredictions["NaiveBayes"].push_back(naiveBayes.predict(fv));
            }
            if (useRandomForest) {
                basePredictions["RandomForest"].push_back(randomForest.predict(fv));
            }
        }
        
        metaLearner.train(basePredictions, groundTruth);
    }
    
    isTrained = true;
}

EnsemblePrediction EnsembleClassifier::predict(const FeatureVector& features) const {
    auto individualPredictions = getIndividualPredictions(features);
    
    if (ensembleStrategy == "stacking") {
        return metaLearner.predict(individualPredictions);
    }
    
    // Simple or weighted voting
    EnsemblePrediction ensemble;
    std::unordered_map<std::string, double> classVotes;
    
    for (const auto& pair : individualPredictions) {
        const std::string& modelName = pair.first;
        const ClassificationResult& prediction = pair.second;
        
        double weight = (ensembleStrategy == "weighted") ? prediction.confidence : 1.0;
        classVotes[prediction.label] += weight;
        ensemble.modelVotes[modelName] = prediction.confidence;
    }
    
    // Find best class
    std::string bestClass;
    double maxVotes = 0.0;
    double totalVotes = 0.0;
    for (const auto& pair : classVotes) {
        totalVotes += pair.second;
        if (pair.second > maxVotes) {
            maxVotes = pair.second;
            bestClass = pair.first;
        }
    }
    
    ensemble.finalLabel = bestClass;
    ensemble.confidence = maxVotes / totalVotes;
    ensemble.weightedVotes = classVotes;
    
    return ensemble;
}

std::unordered_map<std::string, ClassificationResult> EnsembleClassifier::getIndividualPredictions(
    const FeatureVector& features) const {
    
    std::unordered_map<std::string, ClassificationResult> predictions;
    
    if (useNaiveBayes && naiveBayes.isModelTrained()) {
        predictions["NaiveBayes"] = naiveBayes.predict(features);
    }
    
    if (useRandomForest && randomForest.isModelTrained()) {
        predictions["RandomForest"] = randomForest.predict(features);
    }
    
    return predictions;
}

double EnsembleClassifier::evaluate(const std::vector<FeatureVector>& testFeatures) const {
    int correct = 0;
    for (const auto& fv : testFeatures) {
        auto prediction = predict(fv);
        if (prediction.finalLabel == fv.groundTruthLabel) {
            correct++;
        }
    }
    
    return static_cast<double>(correct) / testFeatures.size();
}

std::map<std::pair<std::string, std::string>, int> EnsembleClassifier::getConfusionMatrix(
    const std::vector<FeatureVector>& testFeatures) const {
    
    std::map<std::pair<std::string, std::string>, int> matrix;
    
    for (const auto& fv : testFeatures) {
        auto prediction = predict(fv);
        matrix[{fv.groundTruthLabel, prediction.finalLabel}]++;
    }
    
    return matrix;
}

std::string EnsembleClassifier::getEnsembleInfo() const {
    std::stringstream ss;
    ss << "Ensemble Classifier\n";
    ss << "  Strategy: " << ensembleStrategy << "\n";
    ss << "  Trained: " << (isTrained ? "Yes" : "No") << "\n";
    ss << "  Models:\n";
    if (useNaiveBayes) ss << "    - Naive Bayes\n";
    if (useRandomForest) ss << "    - Random Forest\n";
    return ss.str();
}

// ==================== Feature Extractor ====================

FeatureVector ProposalFeatureExtractor::extractFeatures(
    const std::shared_ptr<Proposal>& proposal,
    bool includeText) {
    
    FeatureVector fv(proposal->getProposalId());
    
    // Numeric features
    fv.features["vote_count"] = static_cast<double>(proposal->getVoteCount());
    fv.features["title_length"] = static_cast<double>(proposal->getTitle().length());
    fv.features["description_length"] = static_cast<double>(proposal->getDescription().length());
    
    // Text tokenization for NLP models
    if (includeText) {
        std::string text = proposal->getTitle() + " " + proposal->getDescription();
        std::istringstream iss(text);
        std::string token;
        while (iss >> token) {
            // Simple tokenization - convert to lowercase
            std::transform(token.begin(), token.end(), token.begin(), ::tolower);
            fv.textTokens.push_back(token);
        }
    }
    
    return fv;
}

std::vector<FeatureVector> ProposalFeatureExtractor::extractBatch(
    const std::vector<std::shared_ptr<Proposal>>& proposals,
    const std::vector<std::string>& labels) {
    
    std::vector<FeatureVector> features;
    
    for (size_t i = 0; i < proposals.size(); i++) {
        FeatureVector fv = extractFeatures(proposals[i]);
        if (i < labels.size()) {
            fv.groundTruthLabel = labels[i];
        }
        features.push_back(fv);
    }
    
    return features;
}

std::vector<std::string> ProposalFeatureExtractor::getFeatureNames() {
    return {"vote_count", "title_length", "description_length"};
}
