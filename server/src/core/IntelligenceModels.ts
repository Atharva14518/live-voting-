import { Proposal } from '../models/types';

// Naive Bayes Classifier for proposal priority
export class NaiveBayesClassifier {
    private classCounts: Map<string, number> = new Map();
    private featureCounts: Map<string, Map<string, number>> = new Map();
    private vocabulary: Set<string> = new Set();
    private totalDocs: number = 0;
    private smoothingAlpha: number = 1.0;

    public train(proposals: Proposal[]): void {
        this.classCounts.clear();
        this.featureCounts.clear();
        this.vocabulary.clear();
        this.totalDocs = proposals.length;

        for (const proposal of proposals) {
            const label = this.getLabel(proposal.voteCount);
            this.classCounts.set(label, (this.classCounts.get(label) || 0) + 1);

            const words = this.tokenize(proposal.title + ' ' + proposal.description);
            for (const word of words) {
                this.vocabulary.add(word);
                if (!this.featureCounts.has(label)) {
                    this.featureCounts.set(label, new Map());
                }
                const labelCounts = this.featureCounts.get(label)!;
                labelCounts.set(word, (labelCounts.get(word) || 0) + 1);
            }
        }
    }

    public predict(proposal: Proposal): { label: string; confidence: number } {
        const words = this.tokenize(proposal.title + ' ' + proposal.description);
        const scores: Map<string, number> = new Map();

        for (const label of this.classCounts.keys()) {
            let logProb = Math.log((this.classCounts.get(label)! + this.smoothingAlpha) / (this.totalDocs + this.smoothingAlpha * this.classCounts.size));

            const labelFeatures = this.featureCounts.get(label) || new Map();
            const totalWordsInClass = Array.from(labelFeatures.values()).reduce((a, b) => a + b, 0);

            for (const word of words) {
                const wordCount = labelFeatures.get(word) || 0;
                logProb += Math.log((wordCount + this.smoothingAlpha) / (totalWordsInClass + this.smoothingAlpha * this.vocabulary.size));
            }
            scores.set(label, logProb);
        }

        let bestLabel = 'low_priority';
        let bestScore = -Infinity;
        for (const [label, score] of scores.entries()) {
            if (score > bestScore) {
                bestScore = score;
                bestLabel = label;
            }
        }

        // Convert log probability to confidence (approximate)
        const totalScore = Array.from(scores.values()).reduce((a, b) => a + Math.exp(b), 0);
        const confidence = Math.exp(bestScore) / totalScore;

        return { label: bestLabel, confidence: Math.min(0.99, confidence) };
    }

    private getLabel(voteCount: number): string {
        if (voteCount >= 30) return 'high_priority';
        if (voteCount >= 15) return 'medium_priority';
        return 'low_priority';
    }

    private tokenize(text: string): string[] {
        return text.toLowerCase().replace(/[^a-z0-9\s]/g, '').split(/\s+/).filter(w => w.length > 2);
    }
}

// Random Forest Classifier (Simplified)
export class RandomForestClassifier {
    private trees: DecisionTree[] = [];
    private numTrees: number;

    constructor(numTrees: number = 10) {
        this.numTrees = numTrees;
    }

    public train(proposals: Proposal[]): void {
        this.trees = [];
        for (let i = 0; i < this.numTrees; i++) {
            const sample = this.bootstrapSample(proposals);
            const tree = new DecisionTree();
            tree.train(sample);
            this.trees.push(tree);
        }
    }

    public predict(proposal: Proposal): { label: string; confidence: number } {
        const votes: Map<string, number> = new Map();

        for (const tree of this.trees) {
            const pred = tree.predict(proposal);
            votes.set(pred, (votes.get(pred) || 0) + 1);
        }

        let bestLabel = 'low_priority';
        let bestVotes = 0;
        for (const [label, count] of votes.entries()) {
            if (count > bestVotes) {
                bestVotes = count;
                bestLabel = label;
            }
        }

        return { label: bestLabel, confidence: bestVotes / this.numTrees };
    }

    private bootstrapSample(data: Proposal[]): Proposal[] {
        const sample: Proposal[] = [];
        for (let i = 0; i < data.length; i++) {
            sample.push(data[Math.floor(Math.random() * data.length)]);
        }
        return sample;
    }
}

// Simple Decision Tree
class DecisionTree {
    private threshold: number = 10;

    public train(proposals: Proposal[]): void {
        // Simple: find optimal vote threshold
        const sorted = proposals.slice().sort((a, b) => a.voteCount - b.voteCount);
        if (sorted.length > 0) {
            this.threshold = sorted[Math.floor(sorted.length / 2)].voteCount;
        }
    }

    public predict(proposal: Proposal): string {
        if (proposal.voteCount >= this.threshold * 2) return 'high_priority';
        if (proposal.voteCount >= this.threshold) return 'medium_priority';
        return 'low_priority';
    }
}

// Ensemble Model combining NaiveBayes and RandomForest
export class EnsembleClassifier {
    private naiveBayes: NaiveBayesClassifier = new NaiveBayesClassifier();
    private randomForest: RandomForestClassifier = new RandomForestClassifier(5);

    public train(proposals: Proposal[]): void {
        this.naiveBayes.train(proposals);
        this.randomForest.train(proposals);
    }

    public predict(proposal: Proposal): {
        naiveBayes: { label: string; confidence: number };
        randomForest: { label: string; confidence: number };
        ensemble: { label: string; confidence: number };
    } {
        const nbResult = this.naiveBayes.predict(proposal);
        const rfResult = this.randomForest.predict(proposal);

        // Simple voting ensemble
        const votes: Map<string, number> = new Map();
        votes.set(nbResult.label, (votes.get(nbResult.label) || 0) + nbResult.confidence);
        votes.set(rfResult.label, (votes.get(rfResult.label) || 0) + rfResult.confidence);

        let bestLabel = 'low_priority';
        let bestScore = 0;
        for (const [label, score] of votes.entries()) {
            if (score > bestScore) {
                bestScore = score;
                bestLabel = label;
            }
        }

        return {
            naiveBayes: nbResult,
            randomForest: rfResult,
            ensemble: { label: bestLabel, confidence: bestScore / 2 }
        };
    }
}
