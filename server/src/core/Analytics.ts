import natural from 'natural';
import { Proposal } from '../models/types';

export class Analytics {
    private static tokenizer = new natural.WordTokenizer();
    private static tfidf = new natural.TfIdf();

    public static calculateSentiment(text: string): number {
        const Analyzer = natural.SentimentAnalyzer;
        const stemmer = natural.PorterStemmer;
        const analyzer = new Analyzer("English", stemmer, "afinn");
        const tokens = this.tokenizer.tokenize(text);
        return analyzer.getSentiment(tokens);
    }

    public static getSimilarProposals(target: Proposal, allProposals: Proposal[]): Proposal[] {
        const similar: { proposal: Proposal; score: number }[] = [];

        // Reset TF-IDF
        this.tfidf = new natural.TfIdf();
        this.tfidf.addDocument(target.title + " " + target.description);

        allProposals.forEach(p => {
            if (p.id === target.id) return;
            this.tfidf.addDocument(p.title + " " + p.description);
        });

        // Calculate similarity (using simple TF-IDF measure for now)
        // Note: natural's TfIdf doesn't give a direct "similarity" between docs easily without more work.
        // Let's use Jaccard Index on tokens for simplicity and robustness as per the C++ implementation.

        const targetTokens = new Set(this.tokenizer.tokenize(target.title + " " + target.description).map(t => t.toLowerCase()));

        allProposals.forEach(p => {
            if (p.id === target.id) return;
            const pTokens = new Set(this.tokenizer.tokenize(p.title + " " + p.description).map(t => t.toLowerCase()));

            const intersection = new Set([...targetTokens].filter(x => pTokens.has(x)));
            const union = new Set([...targetTokens, ...pTokens]);

            const jaccardIndex = intersection.size / union.size;

            if (jaccardIndex > 0.1) { // Threshold
                similar.push({ proposal: p, score: jaccardIndex });
            }
        });

        return similar.sort((a, b) => b.score - a.score).map(s => s.proposal);
    }
}
