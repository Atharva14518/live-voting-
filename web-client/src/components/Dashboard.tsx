import { useEffect, useState } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { ThumbsUp, TrendingUp, AlertTriangle, X, FileText, ShieldCheck, Activity } from 'lucide-react';
import { useAuth } from '../context/AuthContext';

const API_URL = import.meta.env.VITE_API_URL || '${API_URL}';

interface Proposal {
    id: string;
    title: string;
    description: string;
    voteCount: number;
    authorId: string;
    topics: string[];
    sentimentScore: number;
}

interface LogEntry {
    id: string;
    timestamp: string;
    action: string;
    data: string;
    hash: string;
}

export function Dashboard() {
    const { user } = useAuth();
    const [proposals, setProposals] = useState<Proposal[]>([]);
    const [logs, setLogs] = useState<LogEntry[]>([]);
    const [loading, setLoading] = useState(true);
    const [error] = useState('');
    const [showCreateModal, setShowCreateModal] = useState(false);
    const [showLogsModal, setShowLogsModal] = useState(false);

    // Form State
    const [title, setTitle] = useState('');
    const [description, setDescription] = useState('');
    const [topics, setTopics] = useState('');

    const fetchData = async () => {
        try {
            const [propsRes, logsRes] = await Promise.all([
                fetch(`${API_URL}/api/rankings`),
                fetch(`${API_URL}/api/analytics/logs`)
            ]);

            const propsData = await propsRes.json();
            const logsData = await logsRes.json();

            setProposals(propsData);
            setLogs(logsData);
        } catch (err) {
            console.error(err);
            // Don't set error visible to user for background updates
        } finally {
            setLoading(false);
        }
    };

    useEffect(() => {
        fetchData();
        const interval = setInterval(fetchData, 2000); // Faster updates
        return () => clearInterval(interval);
    }, []);

    const handleVote = async (proposalId: string) => {
        try {
            const userId = user?.id || 'USER_DEMO';
            const res = await fetch(`${API_URL}/api/vote`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ userId, proposalId }),
            });

            if (!res.ok) {
                const err = await res.json();
                alert(err.error);
                return;
            }
            fetchData();
        } catch (err) {
            alert('Failed to vote');
        }
    };

    const handleCreateProposal = async (e: React.FormEvent) => {
        e.preventDefault();
        try {
            const res = await fetch(`${API_URL}/api/proposals`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    title,
                    description,
                    authorId: user?.id || 'USER_DEMO',
                    topics: topics.split(',').map(t => t.trim()).filter(t => t)
                }),
            });

            if (!res.ok) throw new Error('Failed to create');

            setShowCreateModal(false);
            setTitle('');
            setDescription('');
            setTopics('');
            fetchData();
        } catch (err) {
            alert('Failed to create proposal');
        }
    };

    if (loading) return <div className="text-center py-20">Loading...</div>;

    return (
        <div className="max-w-7xl mx-auto px-6 py-12" id="dashboard">
            <div className="flex items-center justify-between mb-12">
                <h2 className="text-3xl font-bold text-white">Live Rankings</h2>
                <div className="flex gap-4">
                    <button
                        onClick={() => setShowLogsModal(true)}
                        className="px-6 py-3 bg-white/5 border border-white/10 text-white rounded-lg hover:bg-white/10 transition-colors flex items-center gap-2"
                    >
                        <ShieldCheck className="w-4 h-4" />
                        Audit Logs
                    </button>
                    <button
                        onClick={() => setShowCreateModal(true)}
                        className="px-6 py-3 bg-primary text-white rounded-lg hover:bg-blue-600 transition-colors flex items-center gap-2"
                    >
                        <FileText className="w-4 h-4" />
                        Create Proposal
                    </button>
                </div>
            </div>

            {error && (
                <div className="bg-red-500/10 border border-red-500/20 text-red-400 p-4 rounded-lg mb-8 flex items-center gap-2">
                    <AlertTriangle className="w-5 h-5" />
                    {error}
                </div>
            )}

            <div className="grid gap-6">
                {proposals.map((proposal, index) => (
                    <motion.div
                        key={proposal.id}
                        layoutId={proposal.id}
                        className="glass-card p-6 flex items-start justify-between group hover:border-primary/30 transition-colors"
                    >
                        <div className="flex-1">
                            <div className="flex items-center gap-3 mb-2">
                                <span className="text-2xl font-bold text-white/50">#{index + 1}</span>
                                <h3 className="text-xl font-semibold text-white group-hover:text-primary transition-colors">
                                    {proposal.title}
                                </h3>
                                {proposal.topics.map(topic => (
                                    <span key={topic} className="px-2 py-1 rounded-full bg-white/5 text-xs text-gray-400 border border-white/10">
                                        {topic}
                                    </span>
                                ))}
                            </div>
                            <p className="text-gray-400 mb-4 max-w-2xl">{proposal.description}</p>

                            <div className="flex items-center gap-6 text-sm text-gray-500">
                                <div className="flex items-center gap-2" title="Sentiment Score (AI Analysis)">
                                    <Activity className="w-4 h-4" />
                                    <span className={proposal.sentimentScore > 0 ? "text-green-400" : "text-red-400"}>
                                        Sentiment: {(proposal.sentimentScore * 100).toFixed(0)}%
                                    </span>
                                </div>
                                <div className="flex items-center gap-2">
                                    <TrendingUp className="w-4 h-4" />
                                    <span>Trending Score: {(proposal.voteCount * 1.5).toFixed(1)}</span>
                                </div>
                            </div>
                        </div>

                        <div className="flex flex-col items-center gap-2 ml-6">
                            <button
                                onClick={() => handleVote(proposal.id)}
                                className="w-12 h-12 rounded-xl bg-white/5 hover:bg-primary hover:text-white flex items-center justify-center transition-all border border-white/10 group-hover:border-primary/50"
                            >
                                <ThumbsUp className="w-5 h-5" />
                            </button>
                            <span className="font-bold text-white">{proposal.voteCount}</span>
                        </div>
                    </motion.div>
                ))}

                {proposals.length === 0 && (
                    <div className="text-center py-20 text-gray-500 bg-white/5 rounded-xl border border-white/10">
                        <p className="text-xl mb-2">No active proposals</p>
                        <p className="text-sm">Be the first to create one!</p>
                    </div>
                )}
            </div>

            {/* Create Proposal Modal */}
            <AnimatePresence>
                {showCreateModal && (
                    <div className="fixed inset-0 z-[100] flex items-center justify-center p-4">
                        <motion.div
                            initial={{ opacity: 0 }} animate={{ opacity: 1 }} exit={{ opacity: 0 }}
                            className="absolute inset-0 bg-black/80 backdrop-blur-sm"
                            onClick={() => setShowCreateModal(false)}
                        />
                        <motion.div
                            initial={{ scale: 0.95, opacity: 0 }} animate={{ scale: 1, opacity: 1 }} exit={{ scale: 0.95, opacity: 0 }}
                            className="relative w-full max-w-lg bg-[#0a0a0a] border border-white/10 rounded-2xl p-8 shadow-2xl"
                        >
                            <button onClick={() => setShowCreateModal(false)} className="absolute top-4 right-4 text-gray-400 hover:text-white">
                                <X className="w-5 h-5" />
                            </button>
                            <h3 className="text-2xl font-bold text-white mb-6">New Proposal</h3>
                            <form onSubmit={handleCreateProposal} className="space-y-4">
                                <div>
                                    <label className="block text-sm font-medium text-gray-400 mb-1">Title</label>
                                    <input
                                        type="text" required
                                        value={title} onChange={e => setTitle(e.target.value)}
                                        className="w-full bg-white/5 border border-white/10 rounded-lg px-4 py-2 text-white focus:border-primary focus:outline-none"
                                        placeholder="e.g., Implement Dark Mode"
                                    />
                                </div>
                                <div>
                                    <label className="block text-sm font-medium text-gray-400 mb-1">Description</label>
                                    <textarea
                                        required rows={4}
                                        value={description} onChange={e => setDescription(e.target.value)}
                                        className="w-full bg-white/5 border border-white/10 rounded-lg px-4 py-2 text-white focus:border-primary focus:outline-none"
                                        placeholder="Describe your proposal..."
                                    />
                                </div>
                                <div>
                                    <label className="block text-sm font-medium text-gray-400 mb-1">Topics (comma separated)</label>
                                    <input
                                        type="text"
                                        value={topics} onChange={e => setTopics(e.target.value)}
                                        className="w-full bg-white/5 border border-white/10 rounded-lg px-4 py-2 text-white focus:border-primary focus:outline-none"
                                        placeholder="ui, ux, feature"
                                    />
                                </div>
                                <button type="submit" className="w-full bg-primary text-white font-medium py-3 rounded-lg hover:bg-blue-600 transition-colors">
                                    Submit Proposal
                                </button>
                            </form>
                        </motion.div>
                    </div>
                )}
            </AnimatePresence>

            {/* Audit Log Modal */}
            <AnimatePresence>
                {showLogsModal && (
                    <div className="fixed inset-0 z-[100] flex items-center justify-center p-4">
                        <motion.div
                            initial={{ opacity: 0 }} animate={{ opacity: 1 }} exit={{ opacity: 0 }}
                            className="absolute inset-0 bg-black/80 backdrop-blur-sm"
                            onClick={() => setShowLogsModal(false)}
                        />
                        <motion.div
                            initial={{ scale: 0.95, opacity: 0 }} animate={{ scale: 1, opacity: 1 }} exit={{ scale: 0.95, opacity: 0 }}
                            className="relative w-full max-w-4xl bg-[#0a0a0a] border border-white/10 rounded-2xl p-8 shadow-2xl max-h-[80vh] flex flex-col"
                        >
                            <button onClick={() => setShowLogsModal(false)} className="absolute top-4 right-4 text-gray-400 hover:text-white">
                                <X className="w-5 h-5" />
                            </button>
                            <div className="flex items-center gap-3 mb-6">
                                <ShieldCheck className="w-6 h-6 text-green-400" />
                                <h3 className="text-2xl font-bold text-white">Tamper-Evident Audit Log</h3>
                            </div>

                            <div className="flex-1 overflow-y-auto space-y-2 pr-2 custom-scrollbar">
                                {logs.slice().reverse().map((log) => (
                                    <div key={log.id} className="bg-white/5 border border-white/5 rounded-lg p-4 font-mono text-sm">
                                        <div className="flex justify-between text-gray-500 mb-1 text-xs">
                                            <span>{new Date(log.timestamp).toLocaleString()}</span>
                                            <span>ID: {log.id}</span>
                                        </div>
                                        <div className="flex items-center gap-2 mb-2">
                                            <span className={`px-2 py-0.5 rounded text-xs font-bold ${log.action === 'ABUSE_DETECTED' ? 'bg-red-500/20 text-red-400' :
                                                log.action === 'VOTE' ? 'bg-blue-500/20 text-blue-400' :
                                                    'bg-gray-500/20 text-gray-400'
                                                }`}>
                                                {log.action}
                                            </span>
                                            <span className="text-white">{log.data}</span>
                                        </div>
                                        <div className="text-[10px] text-gray-600 break-all">
                                            Hash: {log.hash}
                                        </div>
                                    </div>
                                ))}
                            </div>
                        </motion.div>
                    </div>
                )}
            </AnimatePresence>
        </div>
    );
}
