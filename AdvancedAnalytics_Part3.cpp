// Part 3 of AdvancedAnalytics.cpp - Time-Based Filtering and Decision Ranking

#include "AdvancedAnalytics.h"
#include "VotingSystem.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

// ==================== TIME-BASED FILTERING ====================

TimeBasedFilter::TimeBasedFilter() {}

std::chrono::system_clock::time_point TimeBasedFilter::parseTimestamp(
    const std::string& timestamp) const {
    return std::chrono::system_clock::now();
}

double TimeBasedFilter::calculateTimeDecay(
    const std::chrono::system_clock::time_point& timestamp,
    double decayFactor) const {
    
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::hours>(now - timestamp);
    double hours = duration.count();
    
    return std::exp(-decayFactor * hours / 24.0);
}

void TimeBasedFilter::registerProposal(const std::string& proposalId, 
                                      const std::string& timestamp) {
    proposalTimestamps[proposalId] = parseTimestamp(timestamp);
}

std::vector<std::string> TimeBasedFilter::filterByTime(
    const std::vector<std::string>& proposalIds,
    const TimeFilter& filter) {
    
    std::vector<std::string> filtered;
    auto now = std::chrono::system_clock::now();
    
    for (const auto& proposalId : proposalIds) {
        auto it = proposalTimestamps.find(proposalId);
        if (it != proposalTimestamps.end()) {
            auto duration = std::chrono::duration_cast<std::chrono::hours>(now - it->second);
            
            if (duration.count() <= filter.timeWindowHours) {
                filtered.push_back(proposalId);
            }
        }
    }
    
    return filtered;
}

double TimeBasedFilter::calculateTimeScore(const std::string& proposalId, 
                                          const TimeFilter& filter) {
    auto it = proposalTimestamps.find(proposalId);
    if (it == proposalTimestamps.end()) {
        return 0.5;
    }
    
    return calculateTimeDecay(it->second, filter.decayFactor);
}

std::vector<std::string> TimeBasedFilter::getRecentProposals(int hours) {
    std::vector<std::string> recent;
    auto now = std::chrono::system_clock::now();
    
    for (const auto& pair : proposalTimestamps) {
        auto duration = std::chrono::duration_cast<std::chrono::hours>(now - pair.second);
        if (duration.count() <= hours) {
            recent.push_back(pair.first);
        }
    }
    
    return recent;
}

std::vector<std::string> TimeBasedFilter::getTrendingProposals(int hours) {
    return getRecentProposals(hours);
}

// ==================== DECISION RANKING ENGINE ====================

DecisionRankingEngine::DecisionRankingEngine() {}

void DecisionRankingEngine::initialize(
    const std::vector<std::shared_ptr<Proposal>>& proposals) {
    
    for (const auto& proposal : proposals) {
        topicAnalyzer.analyzeProposal(proposal);
        timeFilter.registerProposal(proposal->getProposalId(), 
                                    proposal->getCreationTimestamp());
    }
    
    buildSimilarityMatrix(proposals);
}

void DecisionRankingEngine::buildSimilarityMatrix(
    const std::vector<std::shared_ptr<Proposal>>& proposals) {
    
    for (size_t i = 0; i < proposals.size(); ++i) {
        for (size_t j = i + 1; j < proposals.size(); ++j) {
            std::string text1 = proposals[i]->getTitle() + " " + proposals[i]->getDescription();
            std::string text2 = proposals[j]->getTitle() + " " + proposals[j]->getDescription();
            
            double similarity = SimilarityMetrics::combinedSimilarity(text1, text2);
            
            similarityMatrix[proposals[i]->getProposalId()][proposals[j]->getProposalId()] = similarity;
            similarityMatrix[proposals[j]->getProposalId()][proposals[i]->getProposalId()] = similarity;
        }
    }
}

double DecisionRankingEngine::calculateWeightedRelevance(
    const std::shared_ptr<Proposal>& proposal,
    const std::vector<std::string>& coreTopics) {
    
    double totalRelevance = 0.0;
    auto proposalTopics = topicAnalyzer.getProposalTopics(proposal->getProposalId());
    
    for (const auto& topicId : proposalTopics) {
        if (std::find(coreTopics.begin(), coreTopics.end(), topicId) != coreTopics.end()) {
            Topic topic = topicAnalyzer.getTopic(topicId);
            totalRelevance += topic.relevanceScore;
        }
    }
    
    return totalRelevance / std::max(1.0, static_cast<double>(coreTopics.size()));
}

double DecisionRankingEngine::calculatePriorityScore(
    const std::shared_ptr<Proposal>& proposal) {
    
    double voteScore = static_cast<double>(proposal->getVoteCount()) / 100.0;
    double engagementScore = static_cast<double>(proposal->getVoters().size()) / 50.0;
    
    return std::min(1.0, 0.6 * voteScore + 0.4 * engagementScore);
}

double DecisionRankingEngine::calculatePercentile(double score, 
                                                 const std::vector<double>& allScores) {
    if (allScores.empty()) return 50.0;
    
    int countBelow = 0;
    for (double s : allScores) {
        if (s < score) countBelow++;
    }
    
    return (static_cast<double>(countBelow) / allScores.size()) * 100.0;
}

std::vector<DecisionRanking> DecisionRankingEngine::rankDecisionsByTopic(
    const std::string& coreTopicId,
    const std::vector<std::shared_ptr<Proposal>>& proposals,
    const TimeFilter& timeFilter) {
    
    std::vector<DecisionRanking> rankings;
    std::vector<double> allScores;
    
    for (const auto& proposal : proposals) {
        DecisionRanking ranking;
        ranking.proposalId = proposal->getProposalId();
        ranking.title = proposal->getTitle();
        
        std::vector<std::string> coreTopics = {coreTopicId};
        ranking.weightedRelevance = calculateWeightedRelevance(proposal, coreTopics);
        ranking.timeScore = this->timeFilter.calculateTimeScore(proposal->getProposalId(), timeFilter);
        ranking.priorityScore = calculatePriorityScore(proposal);
        
        ranking.combinedScore = 0.4 * ranking.weightedRelevance + 
                               0.3 * ranking.timeScore + 
                               0.3 * ranking.priorityScore;
        
        ranking.matchedTopics = topicAnalyzer.getProposalTopics(proposal->getProposalId());
        
        rankings.push_back(ranking);
        allScores.push_back(ranking.combinedScore);
    }
    
    auto normalizedScores = NormalizationUtils::minMaxNormalize(allScores);
    for (size_t i = 0; i < rankings.size(); ++i) {
        rankings[i].combinedScore = normalizedScores[i];
    }
    
    std::sort(rankings.begin(), rankings.end(), 
             [](const DecisionRanking& a, const DecisionRanking& b) {
                 return a.combinedScore > b.combinedScore;
             });
    
    for (size_t i = 0; i < rankings.size(); ++i) {
        rankings[i].rank = i + 1;
        rankings[i].percentile = calculatePercentile(rankings[i].combinedScore, allScores);
    }
    
    return rankings;
}

std::vector<DecisionRanking> DecisionRankingEngine::getTopDecisions(int n) {
    std::vector<DecisionRanking> topDecisions;
    
    auto tempQueue = rankingQueue;
    for (int i = 0; i < n && !tempQueue.empty(); ++i) {
        topDecisions.push_back(tempQueue.top());
        tempQueue.pop();
    }
    
    return topDecisions;
}

DecisionRanking DecisionRankingEngine::getProposalRanking(const std::string& proposalId) {
    auto tempQueue = rankingQueue;
    
    while (!tempQueue.empty()) {
        auto ranking = tempQueue.top();
        tempQueue.pop();
        
        if (ranking.proposalId == proposalId) {
            return ranking;
        }
    }
    
    return DecisionRanking();
}

double DecisionRankingEngine::getProposalSimilarity(const std::string& proposalId1, 
                                                   const std::string& proposalId2) {
    auto it1 = similarityMatrix.find(proposalId1);
    if (it1 != similarityMatrix.end()) {
        auto it2 = it1->second.find(proposalId2);
        if (it2 != it1->second.end()) {
            return it2->second;
        }
    }
    
    return 0.0;
}

void DecisionRankingEngine::updateRankings(
    const std::vector<std::shared_ptr<Proposal>>& proposals) {
    
    while (!rankingQueue.empty()) {
        rankingQueue.pop();
    }
    
    initialize(proposals);
}

std::string DecisionRankingEngine::getRankingStatistics() {
    std::stringstream ss;
    ss << "\n=== DECISION RANKING STATISTICS ===\n";
    ss << "Total Topics: " << topicAnalyzer.getAllTopics().size() << "\n";
    ss << "Similarity Matrix Size: " << similarityMatrix.size() << "\n";
    ss << "Rankings in Queue: " << rankingQueue.size() << "\n";
    
    return ss.str();
}

// ==================== RANK AND PERCENTILE SYSTEM ====================

RankPercentileSystem::RankPercentileSystem() {}

double RankPercentileSystem::calculatePercentileRank(double score, 
                                                     const std::vector<double>& allScores) {
    if (allScores.empty()) return 50.0;
    
    int countBelow = 0;
    for (double s : allScores) {
        if (s < score) countBelow++;
    }
    
    return (static_cast<double>(countBelow) / allScores.size()) * 100.0;
}

void RankPercentileSystem::updateRankings(
    const std::vector<std::pair<std::string, double>>& proposalScores) {
    
    scores.clear();
    proposalRanks.clear();
    proposalPercentiles.clear();
    
    for (const auto& pair : proposalScores) {
        scores.push_back(pair.second);
    }
    
    std::vector<std::pair<std::string, double>> sorted = proposalScores;
    std::sort(sorted.begin(), sorted.end(),
             [](const auto& a, const auto& b) { return a.second > b.second; });
    
    for (size_t i = 0; i < sorted.size(); ++i) {
        proposalRanks[sorted[i].first] = i + 1;
        proposalPercentiles[sorted[i].first] = calculatePercentileRank(sorted[i].second, scores);
    }
}

int RankPercentileSystem::getRank(const std::string& proposalId) const {
    auto it = proposalRanks.find(proposalId);
    return (it != proposalRanks.end()) ? it->second : -1;
}

double RankPercentileSystem::getPercentile(const std::string& proposalId) const {
    auto it = proposalPercentiles.find(proposalId);
    return (it != proposalPercentiles.end()) ? it->second : 0.0;
}

std::vector<std::string> RankPercentileSystem::getProposalsInPercentileRange(
    double minPercentile, double maxPercentile) {
    
    std::vector<std::string> result;
    
    for (const auto& pair : proposalPercentiles) {
        if (pair.second >= minPercentile && pair.second <= maxPercentile) {
            result.push_back(pair.first);
        }
    }
    
    return result;
}

std::vector<std::string> RankPercentileSystem::getTopPercentProposals(double percentile) {
    return getProposalsInPercentileRange(percentile, 100.0);
}

std::string RankPercentileSystem::getStatistics() const {
    std::stringstream ss;
    ss << "\n=== RANK & PERCENTILE STATISTICS ===\n";
    ss << "Total Proposals Ranked: " << proposalRanks.size() << "\n";
    
    if (!scores.empty()) {
        double mean = NormalizationUtils::calculateMean(scores);
        double stdDev = NormalizationUtils::calculateStdDev(scores, mean);
        
        ss << std::fixed << std::setprecision(3);
        ss << "Mean Score: " << mean << "\n";
        ss << "Std Deviation: " << stdDev << "\n";
        ss << "Min Score: " << *std::min_element(scores.begin(), scores.end()) << "\n";
        ss << "Max Score: " << *std::max_element(scores.begin(), scores.end()) << "\n";
    }
    
    return ss.str();
}
