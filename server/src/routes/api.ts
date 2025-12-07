import express from 'express';
import jwt from 'jsonwebtoken';
import { MaxHeap } from '../core/MaxHeap';
import { AuditLog } from '../core/AuditLog';
import { AntiAbuse } from '../core/AntiAbuse';
import { Analytics } from '../core/Analytics';
import { AuthService } from '../core/AuthService';
import { EnsembleClassifier } from '../core/IntelligenceModels';
import { antiBot } from '../core/AdvancedAntiBot';
import { Proposal, User, Vote } from '../models/types';

const router = express.Router();

// JWT Secret (in production, use environment variable)
const JWT_SECRET = process.env.JWT_SECRET || 'crowddecision_secure_key_2024';
const JWT_EXPIRY = '24h';

// In-memory storage
const maxHeap = new MaxHeap();
const auditLog = new AuditLog();
const users = new Map<string, User>();
const proposals = new Map<string, Proposal>();
const authService = new AuthService(users);
const ensembleClassifier = new EnsembleClassifier();

// Pending CAPTCHA verifications for votes
const pendingVoteCaptchas = new Map<string, { proposalId: string; userId: string; captchaId: string }>();

// Initialize demo users with known credentials
// DEMO ACCOUNTS:
// - demo / demo123
// - admin / admin123
// - test / test123
const demoUsers = [
    { id: 'USER_DEMO', username: 'demo', password: 'demo123', trustScore: 1.0 },
    { id: 'USER_ADMIN', username: 'admin', password: 'admin123', trustScore: 1.0 },
    { id: 'USER_TEST', username: 'test', password: 'test123', trustScore: 0.9 },
];

demoUsers.forEach(u => {
    const user: User = {
        id: u.id,
        username: u.username,
        registeredAt: Date.now(),
        trustScore: u.trustScore,
        voteHistory: [],
        passwordHash: authService.hashPassword(u.password)
    };
    users.set(u.id, user);
});
console.log(`✓ Created ${demoUsers.length} demo users (demo/demo123, admin/admin123, test/test123)`);

// Initialize seed proposals for ML training
const seedProposals = [
    { title: 'Implement Dark Mode for Better UX', description: 'Add a dark theme option to reduce eye strain and improve accessibility for users who prefer darker interfaces.', topics: ['ui', 'accessibility'], voteCount: 45 },
    { title: 'Add Real-time Notifications', description: 'Implement push notifications to alert users about votes, new proposals, and important updates in real-time.', topics: ['feature', 'notifications'], voteCount: 38 },
    { title: 'Mobile App Development', description: 'Create native mobile applications for iOS and Android to reach more users and provide better mobile experience.', topics: ['mobile', 'feature'], voteCount: 52 },
    { title: 'API Rate Limiting Enhancement', description: 'Improve the rate limiting system to better handle high traffic and prevent abuse.', topics: ['security', 'backend'], voteCount: 22 },
    { title: 'Add Export to CSV Feature', description: 'Allow users to export proposal data and analytics to CSV format for further analysis.', topics: ['feature', 'analytics'], voteCount: 15 },
    { title: 'Improve Search Functionality', description: 'Enhance the search with filters, fuzzy matching, and advanced query syntax.', topics: ['feature', 'ux'], voteCount: 28 },
    { title: 'Database Backup Automation', description: 'Implement automated daily backups of all data to prevent data loss.', topics: ['infrastructure', 'backend'], voteCount: 8 },
    { title: 'Add Comment System', description: 'Allow users to comment on proposals to facilitate discussion and feedback.', topics: ['feature', 'social'], voteCount: 35 },
];

seedProposals.forEach((seed, index) => {
    const id = `SEED_PROP_${index + 1}`;
    const proposal: Proposal = {
        id,
        title: seed.title,
        description: seed.description,
        authorId: 'USER_DEMO',
        createdAt: Date.now() - (index * 86400000),
        voteCount: seed.voteCount,
        topics: seed.topics,
        sentimentScore: Analytics.calculateSentiment(seed.title + ' ' + seed.description)
    };
    proposals.set(id, proposal);
    maxHeap.insert(proposal);
});

// Train models with seed data
ensembleClassifier.train(Array.from(proposals.values()));
auditLog.logAction('ML_INIT', `Models trained on ${proposals.size} seed proposals`);
console.log(`✓ ML Models trained on ${proposals.size} seed proposals`);

// Middleware to check rate limit
const rateLimitMiddleware = (req: express.Request, res: express.Response, next: express.NextFunction) => {
    const ip = req.ip || req.socket.remoteAddress || 'unknown';
    const result = antiBot.checkRateLimit(ip);

    res.setHeader('X-RateLimit-Remaining', result.remaining.toString());
    res.setHeader('X-RateLimit-Reset', result.resetIn.toString());

    if (!result.allowed) {
        auditLog.logAction('RATE_LIMITED', `IP ${ip} blocked for excessive requests`);
        return res.status(429).json({
            error: 'Too many requests. Please slow down.',
            retryAfter: Math.ceil(result.resetIn / 1000)
        });
    }
    next();
};

// Apply rate limiting to all routes
router.use(rateLimitMiddleware);

// JWT token generation
const generateToken = (userId: string): string => {
    return jwt.sign({ userId }, JWT_SECRET, { expiresIn: JWT_EXPIRY });
};

// JWT verification middleware
const verifyToken = (req: express.Request, res: express.Response, next: express.NextFunction) => {
    const authHeader = req.headers.authorization;
    if (!authHeader?.startsWith('Bearer ')) {
        return res.status(401).json({ error: 'Authorization required' });
    }

    const token = authHeader.split(' ')[1];
    try {
        const decoded = jwt.verify(token, JWT_SECRET) as { userId: string };
        (req as any).userId = decoded.userId;
        next();
    } catch {
        return res.status(401).json({ error: 'Invalid or expired token' });
    }
};

// ===== AUTH ROUTES =====
router.post('/auth/register', (req, res) => {
    const { username, password, captchaId, captchaAnswer } = req.body;

    if (!username || !password) {
        return res.status(400).json({ error: 'Username and password are required' });
    }

    if (password.length < 6) {
        return res.status(400).json({ error: 'Password must be at least 6 characters' });
    }

    // Require CAPTCHA for registration
    if (!captchaId || captchaAnswer === undefined) {
        const captcha = antiBot.generateCaptcha();
        return res.status(400).json({
            error: 'CAPTCHA required',
            captcha: { id: captcha.id, question: captcha.question }
        });
    }

    const captchaResult = antiBot.verifyCaptcha(captchaId, parseInt(captchaAnswer));
    if (!captchaResult.valid) {
        const newCaptcha = antiBot.generateCaptcha();
        return res.status(400).json({
            error: captchaResult.error,
            captcha: { id: newCaptcha.id, question: newCaptcha.question }
        });
    }

    const result = authService.register(username, password);

    if ('error' in result) {
        return res.status(400).json({ error: result.error });
    }

    const token = generateToken(result.user.id);
    auditLog.logAction('REGISTER', `User registered: ${username} (${result.user.id})`);

    const { passwordHash, ...safeUser } = result.user as any;
    res.json({ user: safeUser, token });
});

router.post('/auth/login', (req, res) => {
    const { username, password } = req.body;

    const result = authService.login(username, password);

    if ('error' in result) {
        return res.status(401).json({ error: result.error });
    }

    const token = generateToken(result.user.id);
    auditLog.logAction('LOGIN', `User logged in: ${username}`);

    const { passwordHash, ...safeUser } = result.user as any;
    res.json({ user: safeUser, token });
});

router.post('/auth/logout', (req, res) => {
    // JWT is stateless, just acknowledge logout
    res.json({ success: true });
});

router.get('/auth/me', verifyToken, (req, res) => {
    const userId = (req as any).userId;
    const user = users.get(userId);

    if (!user) {
        return res.status(404).json({ error: 'User not found' });
    }

    const { passwordHash, ...safeUser } = user as any;
    res.json({ user: safeUser });
});

// ===== CAPTCHA ROUTES =====
router.get('/captcha/generate', (req, res) => {
    const captcha = antiBot.generateCaptcha();
    res.json({ id: captcha.id, question: captcha.question });
});

// ===== PROPOSAL ROUTES =====
router.post('/proposals', (req, res) => {
    const { title, description, authorId, topics } = req.body;

    if (!users.has(authorId)) {
        return res.status(404).json({ error: 'User not found' });
    }

    const id = `PROP_${Date.now()}`;
    const sentimentScore = Analytics.calculateSentiment(title + " " + description);

    const proposal: Proposal = {
        id,
        title,
        description,
        authorId,
        createdAt: Date.now(),
        voteCount: 0,
        topics: topics || [],
        sentimentScore
    };

    proposals.set(id, proposal);
    maxHeap.insert(proposal);
    auditLog.logAction('CREATE_PROPOSAL', `Proposal created: ${title} (${id})`);

    // Retrain models
    if (proposals.size > 2) {
        ensembleClassifier.train(Array.from(proposals.values()));
    }

    res.json(proposal);
});

// Vote with CAPTCHA check
router.post('/vote', (req, res) => {
    const { userId, proposalId, captchaId, captchaAnswer } = req.body;
    const ip = req.ip || req.socket.remoteAddress || 'unknown';

    const user = users.get(userId);
    const proposal = proposals.get(proposalId);

    if (!user || !proposal) {
        return res.status(404).json({ error: 'User or Proposal not found' });
    }

    // Check vote velocity
    const velocityCheck = antiBot.checkVoteVelocity(userId, user.voteHistory);
    if (velocityCheck.suspicious) {
        antiBot.updateReputation(userId, -20);
        auditLog.logAction('ABUSE_DETECTED', `${velocityCheck.reason} for user ${userId}`);
        return res.status(403).json({ error: velocityCheck.reason });
    }

    // Check if CAPTCHA is needed
    if (antiBot.needsCaptcha(userId, ip)) {
        if (!captchaId || captchaAnswer === undefined) {
            const captcha = antiBot.generateCaptcha();
            return res.status(400).json({
                error: 'CAPTCHA required to vote',
                captcha: { id: captcha.id, question: captcha.question }
            });
        }

        const captchaResult = antiBot.verifyCaptcha(captchaId, parseInt(captchaAnswer));
        if (!captchaResult.valid) {
            antiBot.updateReputation(userId, -5);
            const newCaptcha = antiBot.generateCaptcha();
            return res.status(400).json({
                error: captchaResult.error,
                captcha: { id: newCaptcha.id, question: newCaptcha.question }
            });
        }
    }

    // Legacy anti-abuse check
    if (AntiAbuse.isBot(user)) {
        auditLog.logAction('ABUSE_DETECTED', `Bot behavior detected for user ${userId}`);
        return res.status(403).json({ error: 'Suspicious activity detected. Vote rejected.' });
    }

    // Check if already voted
    if (user.voteHistory.some(v => v.proposalId === proposalId)) {
        return res.status(400).json({ error: 'Already voted for this proposal' });
    }

    const vote: Vote = { userId, proposalId, timestamp: Date.now() };
    user.voteHistory.push(vote);

    proposal.voteCount++;
    maxHeap.updateProposal(proposalId, proposal.voteCount);

    // Increase reputation for successful vote
    antiBot.updateReputation(userId, 2);

    auditLog.logAction('VOTE', `User ${user.username} voted for "${proposal.title}"`);

    res.json({ success: true, newCount: proposal.voteCount });
});

router.get('/rankings', (req, res) => {
    const rankings = maxHeap.getAll();
    res.json(rankings);
});

router.get('/proposals/:id', (req, res) => {
    const proposal = proposals.get(req.params.id);
    if (!proposal) return res.status(404).json({ error: 'Proposal not found' });
    res.json(proposal);
});

// ===== ANALYTICS ROUTES =====
router.get('/analytics/logs', (req, res) => {
    res.json(auditLog.getLog());
});

router.get('/analytics/integrity', (req, res) => {
    const isValid = auditLog.verifyIntegrity();
    res.json({ isValid });
});

router.get('/analytics/stats', (req, res) => {
    const allProposals = Array.from(proposals.values());
    const allUsers = Array.from(users.values());
    const colluders = AntiAbuse.detectCollusion(allUsers);
    const antiBotStats = antiBot.getStats();

    res.json({
        totalUsers: allUsers.length,
        totalProposals: allProposals.length,
        totalVotes: allUsers.reduce((sum, u) => sum + u.voteHistory.length, 0),
        averageVotesPerProposal: allProposals.length > 0
            ? allProposals.reduce((sum, p) => sum + p.voteCount, 0) / allProposals.length
            : 0,
        potentialColluders: colluders.length,
        blockedIPs: antiBotStats.blockedIPs,
        topProposal: allProposals.length > 0
            ? allProposals.reduce((top, p) => p.voteCount > top.voteCount ? p : top, allProposals[0])
            : null
    });
});

router.get('/proposals/:id/similar', (req, res) => {
    const proposal = proposals.get(req.params.id);
    if (!proposal) return res.status(404).json({ error: 'Proposal not found' });

    const similar = Analytics.getSimilarProposals(proposal, Array.from(proposals.values()));
    res.json(similar);
});

// ===== INTELLIGENCE ROUTES =====
router.get('/intelligence/classify/:id', (req, res) => {
    const proposal = proposals.get(req.params.id);
    if (!proposal) return res.status(404).json({ error: 'Proposal not found' });

    const result = ensembleClassifier.predict(proposal);
    res.json(result);
});

router.get('/intelligence/classify-all', (req, res) => {
    const allProposals = Array.from(proposals.values());
    const results = allProposals.map(p => ({
        proposal: { id: p.id, title: p.title, voteCount: p.voteCount },
        classification: ensembleClassifier.predict(p)
    }));
    res.json(results);
});

router.post('/intelligence/train', (req, res) => {
    const allProposals = Array.from(proposals.values());
    if (allProposals.length < 3) {
        return res.status(400).json({ error: 'Need at least 3 proposals to train models' });
    }

    ensembleClassifier.train(allProposals);
    auditLog.logAction('ML_TRAIN', `Models retrained on ${allProposals.length} proposals`);

    res.json({ success: true, trainedOn: allProposals.length });
});

// ===== USERS ROUTES =====
router.get('/users', (req, res) => {
    const allUsers = Array.from(users.values()).map(u => {
        const { passwordHash, ...safeUser } = u as any;
        return { ...safeUser, reputation: antiBot.getReputation(u.id) };
    });
    res.json(allUsers);
});

export default router;
