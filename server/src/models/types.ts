export interface User {
    id: string;
    username: string;
    registeredAt: number;
    trustScore: number;
    voteHistory: Vote[];
    passwordHash?: string;
}

export interface Proposal {
    id: string;
    title: string;
    description: string;
    authorId: string;
    createdAt: number;
    voteCount: number;
    topics: string[];
    sentimentScore: number;
}

export interface Vote {
    userId: string;
    proposalId: string;
    timestamp: number;
}

export interface LogEntry {
    id: string;
    timestamp: string;
    action: string;
    data: string;
    hash: string;
    previousHash: string;
}
