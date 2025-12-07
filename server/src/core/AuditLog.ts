import crypto from 'crypto';
import { LogEntry } from '../models/types';

export class AuditLog {
    private chain: LogEntry[] = [];

    constructor() {
        // Genesis block
        this.logAction('SYSTEM', 'System Initialized');
    }

    public logAction(action: string, data: string): LogEntry {
        const previousHash = this.chain.length > 0 ? this.chain[this.chain.length - 1].hash : '0';
        const timestamp = new Date().toISOString();
        const id = `LOG_${Date.now()}_${Math.floor(Math.random() * 1000)}`;

        const entryData = `${id}${timestamp}${action}${data}${previousHash}`;
        const hash = crypto.createHash('sha256').update(entryData).digest('hex');

        const entry: LogEntry = {
            id,
            timestamp,
            action,
            data,
            hash,
            previousHash
        };

        this.chain.push(entry);
        return entry;
    }

    public getLog(): LogEntry[] {
        return [...this.chain];
    }

    public verifyIntegrity(): boolean {
        for (let i = 1; i < this.chain.length; i++) {
            const current = this.chain[i];
            const previous = this.chain[i - 1];

            if (current.previousHash !== previous.hash) {
                return false;
            }

            const entryData = `${current.id}${current.timestamp}${current.action}${current.data}${current.previousHash}`;
            const hash = crypto.createHash('sha256').update(entryData).digest('hex');

            if (hash !== current.hash) {
                return false;
            }
        }
        return true;
    }
}
