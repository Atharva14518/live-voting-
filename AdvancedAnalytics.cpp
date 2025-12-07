#include "AdvancedAnalytics.h"
#include "VotingSystem.h"
#include "IntelligenceEngine.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <chrono>

// ==================== NORMALIZATION UTILITIES ====================

std::vector<double> NormalizationUtils::minMaxNormalize(const std::vector<double>& values) {
    if (values.empty()) return {};
    
    double minVal = *std::min_element(values.begin(), values.end());
    double maxVal = *std::max_element(values.begin(), values.end());
    
    std::vector<double> normalized;
    normalized.reserve(values.size());
    
    if (maxVal - minVal < 1e-10) {
        return std::vector<double>(values.size(), 0.5);
    }
    
    for (double val : values) {
        normalized.push_back((val - minVal) / (maxVal - minVal));
    }
    
    return normalized;
}

std::vector<double> NormalizationUtils::zScoreStandardize(const std::vector<double>& values) {
    if (values.empty()) return {};
    
    double mean = calculateMean(values);
    double stdDev = calculateStdDev(values, mean);
    
    std::vector<double> standardized;
    standardized.reserve(values.size());
    
    if (stdDev < 1e-10) {
        return std::vector<double>(values.size(), 0.0);
    }
    
    for (double val : values) {
        standardized.push_back((val - mean) / stdDev);
    }
    
    return standardized;
}

double NormalizationUtils::calculateMean(const std::vector<double>& values) {
    if (values.empty()) return 0.0;
    return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
}

double NormalizationUtils::calculateStdDev(const std::vector<double>& values, double mean) {
    if (values.empty()) return 0.0;
    
    double sumSquaredDiff = 0.0;
    for (double val : values) {
        double diff = val - mean;
        sumSquaredDiff += diff * diff;
    }
    
    return std::sqrt(sumSquaredDiff / values.size());
}

// ==================== ENHANCED SIMILARITY METRICS ====================

double SimilarityMetrics::jaccardSimilarity(const std::set<std::string>& set1, 
                                           const std::set<std::string>& set2) {
    if (set1.empty() && set2.empty()) return 1.0;
    if (set1.empty() || set2.empty()) return 0.0;
    
    std::set<std::string> intersection;
    std::set_intersection(set1.begin(), set1.end(), 
                         set2.begin(), set2.end(),
                         std::inserter(intersection, intersection.begin()));
    
    std::set<std::string> unionSet;
    std::set_union(set1.begin(), set1.end(), 
                  set2.begin(), set2.end(),
                  std::inserter(unionSet, unionSet.begin()));
    
    return static_cast<double>(intersection.size()) / unionSet.size();
}

double SimilarityMetrics::cosineSimilarity(
    const std::unordered_map<std::string, double>& vec1,
    const std::unordered_map<std::string, double>& vec2) {
    
    if (vec1.empty() || vec2.empty()) return 0.0;
    
    double dotProduct = 0.0;
    double norm1 = 0.0;
    double norm2 = 0.0;
    
    for (const auto& pair : vec1) {
        norm1 += pair.second * pair.second;
        
        auto it = vec2.find(pair.first);
        if (it != vec2.end()) {
            dotProduct += pair.second * it->second;
        }
    }
    
    for (const auto& pair : vec2) {
        norm2 += pair.second * pair.second;
    }
    
    norm1 = std::sqrt(norm1);
    norm2 = std::sqrt(norm2);
    
    if (norm1 < 1e-10 || norm2 < 1e-10) return 0.0;
    
    return dotProduct / (norm1 * norm2);
}

double SimilarityMetrics::combinedSimilarity(const std::string& text1, 
                                            const std::string& text2,
                                            double jaccardWeight, 
                                            double cosineWeight) {
    auto tokens1 = NLPUtils::tokenize(text1);
    auto tokens2 = NLPUtils::tokenize(text2);
    
    std::set<std::string> set1(tokens1.begin(), tokens1.end());
    std::set<std::string> set2(tokens2.begin(), tokens2.end());
    double jaccardSim = jaccardSimilarity(set1, set2);
    
    std::unordered_map<std::string, double> vec1, vec2;
    for (const auto& token : tokens1) vec1[token]++;
    for (const auto& token : tokens2) vec2[token]++;
    double cosineSim = cosineSimilarity(vec1, vec2);
    
    return jaccardWeight * jaccardSim + cosineWeight * cosineSim;
}

std::unordered_map<std::string, double> SimilarityMetrics::createTFIDFVector(
    const std::string& text,
    const std::vector<std::string>& corpus) {
    
    std::unordered_map<std::string, double> tfidf;
    auto tokens = NLPUtils::removeStopWords(NLPUtils::tokenize(text));
    
    std::unordered_map<std::string, int> termFreq;
    for (const auto& token : tokens) termFreq[token]++;
    
    std::unordered_map<std::string, int> docFreq;
    for (const auto& doc : corpus) {
        std::set<std::string> uniqueTokens;
        auto docTokens = NLPUtils::removeStopWords(NLPUtils::tokenize(doc));
        for (const auto& token : docTokens) uniqueTokens.insert(token);
        for (const auto& token : uniqueTokens) docFreq[token]++;
    }
    
    int totalDocs = corpus.size();
    for (const auto& pair : termFreq) {
        double tf = static_cast<double>(pair.second) / tokens.size();
        double idf = std::log(static_cast<double>(totalDocs) / (1 + docFreq[pair.first]));
        tfidf[pair.first] = tf * idf;
    }
    
    return tfidf;
}

// Remaining implementations in separate file...
