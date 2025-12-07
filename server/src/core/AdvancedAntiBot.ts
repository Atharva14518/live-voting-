import crypto from 'crypto';

interface RateLimitEntry {
    count: number;
    firstRequest: number;
    blocked: boolean;
    blockUntil: number;
}

interface CaptchaChallenge {
    id: string;
    question: string;
    answer: number;
    createdAt: number;
    attempts: number;
}

export class AdvancedAntiBot {
    // Rate limiting: IP -> request count per window
    private rateLimits: Map<string, RateLimitEntry> = new Map();

    // CAPTCHA challenges: challengeId -> challenge
    private captchaChallenges: Map<string, CaptchaChallenge> = new Map();

    // User reputation scores
    private reputationScores: Map<string, number> = new Map();

    // Suspicious IPs (detected bots)
    private suspiciousIPs: Set<string> = new Set();

    // Config - Production-friendly settings
    private readonly RATE_LIMIT_WINDOW = 60000; // 1 minute
    private readonly MAX_REQUESTS_PER_WINDOW = 1000; // Very high for production frontend polling
    private readonly BLOCK_DURATION = 30000; // 30 seconds block (reduced from 5 minutes)
    private readonly CAPTCHA_EXPIRY = 120000; // 2 minutes
    private readonly MAX_CAPTCHA_ATTEMPTS = 3;

    // Check rate limit for an IP
    public checkRateLimit(ip: string): { allowed: boolean; remaining: number; resetIn: number } {
        const now = Date.now();
        let entry = this.rateLimits.get(ip);

        // Check if blocked
        if (entry?.blocked && entry.blockUntil > now) {
            return { allowed: false, remaining: 0, resetIn: entry.blockUntil - now };
        }

        // Reset if window expired
        if (!entry || now - entry.firstRequest > this.RATE_LIMIT_WINDOW) {
            entry = { count: 0, firstRequest: now, blocked: false, blockUntil: 0 };
        }

        entry.count++;

        // Block if exceeded
        if (entry.count > this.MAX_REQUESTS_PER_WINDOW) {
            entry.blocked = true;
            entry.blockUntil = now + this.BLOCK_DURATION;
            this.suspiciousIPs.add(ip);
            this.rateLimits.set(ip, entry);
            return { allowed: false, remaining: 0, resetIn: this.BLOCK_DURATION };
        }

        this.rateLimits.set(ip, entry);
        return {
            allowed: true,
            remaining: this.MAX_REQUESTS_PER_WINDOW - entry.count,
            resetIn: this.RATE_LIMIT_WINDOW - (now - entry.firstRequest)
        };
    }

    // Generate a math CAPTCHA challenge
    public generateCaptcha(): { id: string; question: string } {
        const operations = ['+', '-', '*'];
        const op = operations[Math.floor(Math.random() * operations.length)];
        let a: number, b: number, answer: number;

        switch (op) {
            case '+':
                a = Math.floor(Math.random() * 50) + 10;
                b = Math.floor(Math.random() * 50) + 10;
                answer = a + b;
                break;
            case '-':
                a = Math.floor(Math.random() * 50) + 50;
                b = Math.floor(Math.random() * 40) + 10;
                answer = a - b;
                break;
            case '*':
                a = Math.floor(Math.random() * 12) + 2;
                b = Math.floor(Math.random() * 12) + 2;
                answer = a * b;
                break;
            default:
                a = 5; b = 5; answer = 10;
        }

        const id = crypto.randomBytes(16).toString('hex');
        const question = `What is ${a} ${op} ${b}?`;

        this.captchaChallenges.set(id, {
            id,
            question,
            answer,
            createdAt: Date.now(),
            attempts: 0
        });

        return { id, question };
    }

    // Verify CAPTCHA answer
    public verifyCaptcha(id: string, userAnswer: number): { valid: boolean; error?: string } {
        const challenge = this.captchaChallenges.get(id);

        if (!challenge) {
            return { valid: false, error: 'CAPTCHA expired or not found. Please request a new one.' };
        }

        // Check expiry
        if (Date.now() - challenge.createdAt > this.CAPTCHA_EXPIRY) {
            this.captchaChallenges.delete(id);
            return { valid: false, error: 'CAPTCHA expired. Please request a new one.' };
        }

        // Check attempts
        challenge.attempts++;
        if (challenge.attempts > this.MAX_CAPTCHA_ATTEMPTS) {
            this.captchaChallenges.delete(id);
            return { valid: false, error: 'Too many attempts. Please request a new CAPTCHA.' };
        }

        // Verify answer
        if (challenge.answer !== userAnswer) {
            return { valid: false, error: `Incorrect answer. ${this.MAX_CAPTCHA_ATTEMPTS - challenge.attempts} attempts remaining.` };
        }

        // Success - delete used CAPTCHA
        this.captchaChallenges.delete(id);
        return { valid: true };
    }

    // Check vote velocity (too many votes too fast)
    public checkVoteVelocity(userId: string, recentVotes: { timestamp: number }[]): { suspicious: boolean; reason?: string } {
        const now = Date.now();
        const lastMinute = recentVotes.filter(v => now - v.timestamp < 60000);
        const lastHour = recentVotes.filter(v => now - v.timestamp < 3600000);

        // Too many votes per minute (likely bot)
        if (lastMinute.length >= 10) {
            return { suspicious: true, reason: 'Vote velocity too high (>10/min)' };
        }

        // Too many votes per hour
        if (lastHour.length >= 50) {
            return { suspicious: true, reason: 'Excessive voting activity (>50/hour)' };
        }

        // Check for perfectly regular intervals (bot pattern)
        if (lastMinute.length >= 3) {
            const intervals = [];
            for (let i = 1; i < lastMinute.length; i++) {
                intervals.push(lastMinute[i].timestamp - lastMinute[i - 1].timestamp);
            }
            const avgInterval = intervals.reduce((a, b) => a + b, 0) / intervals.length;
            const variance = intervals.reduce((sum, i) => sum + Math.pow(i - avgInterval, 2), 0) / intervals.length;

            // If variance is very low, votes are too regular (bot-like)
            if (variance < 100) { // Less than 100ms variance
                return { suspicious: true, reason: 'Voting pattern too regular (automated)' };
            }
        }

        return { suspicious: false };
    }

    // Update reputation score
    public updateReputation(userId: string, delta: number): number {
        const current = this.reputationScores.get(userId) || 100;
        const newScore = Math.max(0, Math.min(200, current + delta));
        this.reputationScores.set(userId, newScore);
        return newScore;
    }

    // Get reputation
    public getReputation(userId: string): number {
        return this.reputationScores.get(userId) || 100;
    }

    // Check if user needs CAPTCHA
    public needsCaptcha(userId: string, ip: string): boolean {
        const reputation = this.getReputation(userId);

        // Low reputation users always need CAPTCHA
        if (reputation < 50) return true;

        // Suspicious IPs need CAPTCHA
        if (this.suspiciousIPs.has(ip)) return true;

        // Random 10% of votes need CAPTCHA (honeypot)
        if (Math.random() < 0.1 && reputation < 150) return true;

        return false;
    }

    // Get system stats
    public getStats() {
        return {
            blockedIPs: this.suspiciousIPs.size,
            activeCaptchas: this.captchaChallenges.size,
            trackedUsers: this.reputationScores.size
        };
    }
}

// Singleton instance
export const antiBot = new AdvancedAntiBot();
