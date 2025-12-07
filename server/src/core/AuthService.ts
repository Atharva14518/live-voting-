import crypto from 'crypto';
import { User } from '../models/types';

// Simple in-memory session store
const sessions = new Map<string, string>(); // sessionId -> userId

export class AuthService {
    private users: Map<string, User>;

    constructor(users: Map<string, User>) {
        this.users = users;
    }

    public hashPassword(password: string): string {
        return crypto.createHash('sha256').update(password).digest('hex');
    }

    public register(username: string, password: string): { user: User; sessionId: string } | { error: string } {
        // Check if username exists
        for (const user of this.users.values()) {
            if (user.username === username) {
                return { error: 'Username already exists' };
            }
        }

        const id = `USER_${Date.now()}_${Math.floor(Math.random() * 1000)}`;
        const hashedPassword = this.hashPassword(password);

        const user: User = {
            id,
            username,
            registeredAt: Date.now(),
            trustScore: 1.0,
            voteHistory: [],
            passwordHash: hashedPassword
        };

        this.users.set(id, user);

        const sessionId = this.createSession(id);
        return { user, sessionId };
    }

    public login(username: string, password: string): { user: User; sessionId: string } | { error: string } {
        const hashedPassword = this.hashPassword(password);

        for (const user of this.users.values()) {
            if (user.username === username) {
                if ((user as any).passwordHash === hashedPassword) {
                    const sessionId = this.createSession(user.id);
                    return { user, sessionId };
                } else {
                    return { error: 'Invalid password' };
                }
            }
        }
        return { error: 'User not found' };
    }

    public createSession(userId: string): string {
        const sessionId = crypto.randomBytes(32).toString('hex');
        sessions.set(sessionId, userId);
        return sessionId;
    }

    public validateSession(sessionId: string): User | null {
        const userId = sessions.get(sessionId);
        if (!userId) return null;
        return this.users.get(userId) || null;
    }

    public logout(sessionId: string): void {
        sessions.delete(sessionId);
    }
}
