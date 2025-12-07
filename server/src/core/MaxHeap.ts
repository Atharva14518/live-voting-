import { Proposal } from '../models/types';

export class MaxHeap {
    private heap: Proposal[] = [];

    constructor() { }

    public insert(proposal: Proposal): void {
        this.heap.push(proposal);
        this.bubbleUp(this.heap.length - 1);
    }

    public extractMax(): Proposal | undefined {
        if (this.heap.length === 0) return undefined;
        if (this.heap.length === 1) return this.heap.pop();

        const max = this.heap[0];
        this.heap[0] = this.heap.pop()!;
        this.bubbleDown(0);
        return max;
    }

    public peek(): Proposal | undefined {
        return this.heap[0];
    }

    public updateProposal(proposalId: string, newVoteCount: number): void {
        const index = this.heap.findIndex(p => p.id === proposalId);
        if (index !== -1) {
            const oldVoteCount = this.heap[index].voteCount;
            this.heap[index].voteCount = newVoteCount;
            if (newVoteCount > oldVoteCount) {
                this.bubbleUp(index);
            } else {
                this.bubbleDown(index);
            }
        }
    }

    public getAll(): Proposal[] {
        // Return a sorted copy without modifying the heap
        return [...this.heap].sort((a, b) => b.voteCount - a.voteCount);
    }

    private bubbleUp(index: number): void {
        while (index > 0) {
            const parentIndex = Math.floor((index - 1) / 2);
            if (this.heap[index].voteCount <= this.heap[parentIndex].voteCount) break;
            [this.heap[index], this.heap[parentIndex]] = [this.heap[parentIndex], this.heap[index]];
            index = parentIndex;
        }
    }

    private bubbleDown(index: number): void {
        while (true) {
            let largest = index;
            const left = 2 * index + 1;
            const right = 2 * index + 2;

            if (left < this.heap.length && this.heap[left].voteCount > this.heap[largest].voteCount) {
                largest = left;
            }
            if (right < this.heap.length && this.heap[right].voteCount > this.heap[largest].voteCount) {
                largest = right;
            }

            if (largest === index) break;

            [this.heap[index], this.heap[largest]] = [this.heap[largest], this.heap[index]];
            index = largest;
        }
    }
}
