// Part 2 of AdvancedAnalytics.cpp - Topic Analysis and Classification

#include "AdvancedAnalytics.h"
#include "VotingSystem.h"
#include "IntelligenceEngine.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// ==================== TOPIC ANALYSIS ====================

TopicAnalyzer::TopicAnalyzer() {
    addTopic("TECH", "Technology", {"technology", "software", "hardware", "digital", "innovation", "ai", "machine", "learning"});
    addTopic("ENV", "Environment", {"environment", "climate", "sustainability", "green", "renewable", "pollution", "conservation"});
    addTopic("EDU", "Education", {"education", "school", "university", "learning", "teaching", "student", "academic"});
    addTopic("HEALTH", "Healthcare", {"health", "medical", "hospital", "doctor", "patient", "treatment", "medicine"});
    addTopic("ECON", "Economy", {"economy", "financial", "business", "market", "trade", "investment", "budget"});
}

void TopicAnalyzer::addTopic(const std::string& topicId, const std::string& name, 
                            const std::vector<std::string>& keywords) {
    Topic topic(topicId, name);
    topic.keywords = keywords;
    std::sort(topic.keywords.begin(), topic.keywords.end());
    
    for (const auto& keyword : keywords) {
        topic.keywordWeights[keyword] = 1.0;
    }
    
    topics[topicId] = topic;
}

void TopicAnalyzer::decomposeTopicIntoSubTopics(const std::string& topicId, 
                                                const std::vector<std::string>& subTopics) {
    auto it = topics.find(topicId);
    if (it != topics.end()) {
        it->second.subTopics = subTopics;
    }
}

bool TopicAnalyzer::keywordExists(const std::vector<std::string>& sortedKeywords, 
                                 const std::string& keyword) const {
    return std::binary_search(sortedKeywords.begin(), sortedKeywords.end(), 
                             NLPUtils::toLowerCase(keyword));
}

std::vector<std::string> TopicAnalyzer::extractTopicsFromText(const std::string& text) {
    std::vector<std::string> matchedTopics;
    auto tokens = NLPUtils::removeStopWords(NLPUtils::tokenize(text));
    
    for (const auto& pair : topics) {
        const Topic& topic = pair.second;
        int matchCount = 0;
        
        for (const auto& token : tokens) {
            if (keywordExists(topic.keywords, token)) {
                matchCount++;
            }
        }
        
        if (matchCount >= 2) {
            matchedTopics.push_back(topic.topicId);
        }
    }
    
    return matchedTopics;
}

double TopicAnalyzer::calculateTopicRelevance(const std::string& proposalText, 
                                             const Topic& topic) {
    auto tokens = NLPUtils::removeStopWords(NLPUtils::tokenize(proposalText));
    std::set<std::string> tokenSet(tokens.begin(), tokens.end());
    std::set<std::string> keywordSet(topic.keywords.begin(), topic.keywords.end());
    
    double jaccardSim = SimilarityMetrics::jaccardSimilarity(tokenSet, keywordSet);
    
    double weightedScore = 0.0;
    int matchCount = 0;
    
    for (const auto& token : tokens) {
        auto it = topic.keywordWeights.find(token);
        if (it != topic.keywordWeights.end()) {
            weightedScore += it->second;
            matchCount++;
        }
    }
    
    if (matchCount > 0) {
        weightedScore /= matchCount;
    }
    
    return 0.6 * jaccardSim + 0.4 * weightedScore;
}

void TopicAnalyzer::analyzeProposal(const std::shared_ptr<Proposal>& proposal) {
    std::string proposalText = proposal->getTitle() + " " + proposal->getDescription();
    std::string proposalId = proposal->getProposalId();
    
    std::vector<std::string> matchedTopics = extractTopicsFromText(proposalText);
    proposalTopics[proposalId] = matchedTopics;
    
    for (const auto& topicId : matchedTopics) {
        auto it = topics.find(topicId);
        if (it != topics.end()) {
            it->second.relevanceScore = calculateTopicRelevance(proposalText, it->second);
        }
    }
}

std::vector<std::string> TopicAnalyzer::getProposalTopics(const std::string& proposalId) const {
    auto it = proposalTopics.find(proposalId);
    return (it != proposalTopics.end()) ? it->second : std::vector<std::string>();
}

std::vector<std::string> TopicAnalyzer::getProposalsForTopic(const std::string& topicId) const {
    std::vector<std::string> proposals;
    
    for (const auto& pair : proposalTopics) {
        const auto& topics = pair.second;
        if (std::find(topics.begin(), topics.end(), topicId) != topics.end()) {
            proposals.push_back(pair.first);
        }
    }
    
    return proposals;
}

bool TopicAnalyzer::searchKeywordInTopic(const std::string& topicId, 
                                        const std::string& keyword) const {
    auto it = topics.find(topicId);
    if (it == topics.end()) return false;
    
    return keywordExists(it->second.keywords, keyword);
}

Topic TopicAnalyzer::getTopic(const std::string& topicId) const {
    auto it = topics.find(topicId);
    return (it != topics.end()) ? it->second : Topic();
}

std::vector<Topic> TopicAnalyzer::getAllTopics() const {
    std::vector<Topic> allTopics;
    for (const auto& pair : topics) {
        allTopics.push_back(pair.second);
    }
    return allTopics;
}

// ==================== LOGISTIC REGRESSION CLASSIFIER ====================

LogisticRegressionClassifier::LogisticRegressionClassifier(double lr, int maxIter)
    : learningRate(lr), maxIterations(maxIter) {
    
    featureNames = {
        "vote_count", "title_length", "description_length", 
        "keyword_density", "sentiment_score", "time_factor"
    };
}

double LogisticRegressionClassifier::sigmoid(double z) const {
    return 1.0 / (1.0 + std::exp(-z));
}

std::vector<double> LogisticRegressionClassifier::extractFeatures(
    const std::shared_ptr<Proposal>& proposal) {
    
    std::vector<double> features;
    
    features.push_back(static_cast<double>(proposal->getVoteCount()) / 100.0);
    features.push_back(static_cast<double>(proposal->getTitle().length()) / 100.0);
    features.push_back(static_cast<double>(proposal->getDescription().length()) / 500.0);
    
    auto tokens = NLPUtils::removeStopWords(
        NLPUtils::tokenize(proposal->getTitle() + " " + proposal->getDescription()));
    double keywordDensity = static_cast<double>(tokens.size()) / 
                           (proposal->getTitle().length() + proposal->getDescription().length());
    features.push_back(keywordDensity * 100.0);
    
    features.push_back(0.5);
    features.push_back(0.5);
    
    return features;
}

double LogisticRegressionClassifier::calculateProbability(
    const std::vector<double>& features, 
    const std::string& label) const {
    
    auto it = weights.find(label);
    if (it == weights.end() || it->second.size() != features.size()) {
        return 0.5;
    }
    
    double z = 0.0;
    for (size_t i = 0; i < features.size(); ++i) {
        z += features[i] * it->second[i];
    }
    
    return sigmoid(z);
}

void LogisticRegressionClassifier::train(
    const std::vector<std::shared_ptr<Proposal>>& proposals,
    const std::vector<std::string>& labels) {
    
    if (proposals.size() != labels.size()) return;
    
    std::set<std::string> uniqueLabels(labels.begin(), labels.end());
    for (const auto& label : uniqueLabels) {
        weights[label] = std::vector<double>(featureNames.size(), 0.0);
    }
    
    for (int iter = 0; iter < maxIterations; ++iter) {
        for (size_t i = 0; i < proposals.size(); ++i) {
            auto features = extractFeatures(proposals[i]);
            const std::string& trueLabel = labels[i];
            
            for (auto& weightPair : weights) {
                const std::string& label = weightPair.first;
                double predicted = calculateProbability(features, label);
                double actual = (label == trueLabel) ? 1.0 : 0.0;
                double error = actual - predicted;
                
                for (size_t j = 0; j < features.size(); ++j) {
                    weightPair.second[j] += learningRate * error * features[j];
                }
            }
        }
    }
}

ClassificationLabel LogisticRegressionClassifier::classify(
    const std::shared_ptr<Proposal>& proposal) {
    
    auto features = extractFeatures(proposal);
    
    std::string bestLabel;
    double maxProb = 0.0;
    
    for (const auto& pair : weights) {
        double prob = calculateProbability(features, pair.first);
        if (prob > maxProb) {
            maxProb = prob;
            bestLabel = pair.first;
        }
    }
    
    ClassificationLabel result(bestLabel, maxProb);
    result.features = featureNames;
    
    return result;
}

std::unordered_map<std::string, double> LogisticRegressionClassifier::getClassProbabilities(
    const std::shared_ptr<Proposal>& proposal) {
    
    auto features = extractFeatures(proposal);
    std::unordered_map<std::string, double> probabilities;
    
    for (const auto& pair : weights) {
        probabilities[pair.first] = calculateProbability(features, pair.first);
    }
    
    return probabilities;
}

void LogisticRegressionClassifier::addFeature(const std::string& featureName) {
    featureNames.push_back(featureName);
    
    for (auto& pair : weights) {
        pair.second.push_back(0.0);
    }
}
