import { User, Vote } from '../models/types';

export class AntiAbuse {
    private static RATE_LIMIT_WINDOW = 60 * 1000; // 1 minute
    private static MAX_VOTES_PER_WINDOW = 5;

    public static isBot(user: User): boolean {
        const now = Date.now();
        const recentVotes = user.voteHistory.filter(v => now - v.timestamp < this.RATE_LIMIT_WINDOW);

        if (recentVotes.length > this.MAX_VOTES_PER_WINDOW) {
            return true;
        }

        // Check for inhuman speed (less than 500ms between votes)
        for (let i = 1; i < recentVotes.length; i++) {
            if (recentVotes[i].timestamp - recentVotes[i - 1].timestamp < 500) {
                return true;
            }
        }

        return false;
    }

    public static detectCollusion(users: User[]): string[] {
        // Simplified collusion detection: Find groups of users who voted for the exact same proposals in the same order
        const votingPatterns = new Map<string, string[]>();

        for (const user of users) {
            if (user.voteHistory.length < 3) continue; // Need at least 3 votes to establish a pattern

            const pattern = user.voteHistory.map(v => v.proposalId).join(',');
            if (!votingPatterns.has(pattern)) {
                votingPatterns.set(pattern, []);
            }
            votingPatterns.get(pattern)!.push(user.id);
        }

        const colludingUsers: string[] = [];
        for (const [pattern, userIds] of votingPatterns.entries()) {
            if (userIds.length > 2) { // More than 2 users with exact same pattern
                colludingUsers.push(...userIds);
            }
        }

        return colludingUsers;
    }
}
