import { useState, useEffect } from 'react';
import { motion } from 'framer-motion';
import { Brain, BarChart3, RefreshCw, AlertTriangle, CheckCircle, XCircle } from 'lucide-react';

interface Classification {
    label: string;
    confidence: number;
}

interface ProposalClassification {
    proposal: { id: string; title: string; voteCount: number };
    classification: {
        naiveBayes: Classification;
        randomForest: Classification;
        ensemble: Classification;
    };
}

interface Stats {
    totalUsers: number;
    totalProposals: number;
    totalVotes: number;
    averageVotesPerProposal: number;
    potentialColluders: number;
    topProposal: { title: string; voteCount: number } | null;
}

export function IntelligenceDashboard() {
    const [classifications, setClassifications] = useState<ProposalClassification[]>([]);
    const [stats, setStats] = useState<Stats | null>(null);
    const [loading, setLoading] = useState(true);
    const [training, setTraining] = useState(false);

    const fetchData = async () => {
        try {
            const [classRes, statsRes] = await Promise.all([
                fetch('http://localhost:3000/api/intelligence/classify-all'),
                fetch('http://localhost:3000/api/analytics/stats')
            ]);
            const classData = await classRes.json();
            const statsData = await statsRes.json();
            setClassifications(classData);
            setStats(statsData);
        } catch (err) {
            console.error(err);
        } finally {
            setLoading(false);
        }
    };

    useEffect(() => {
        fetchData();
        const interval = setInterval(fetchData, 5000);
        return () => clearInterval(interval);
    }, []);

    const handleRetrain = async () => {
        setTraining(true);
        try {
            await fetch('http://localhost:3000/api/intelligence/train', { method: 'POST' });
            await fetchData();
        } catch (err) {
            alert('Training failed');
        } finally {
            setTraining(false);
        }
    };

    const getPriorityColor = (label: string) => {
        switch (label) {
            case 'high_priority': return 'text-green-400 bg-green-500/20';
            case 'medium_priority': return 'text-yellow-400 bg-yellow-500/20';
            default: return 'text-gray-400 bg-gray-500/20';
        }
    };

    if (loading) return <div className="text-center py-10">Loading intelligence data...</div>;

    return (
        <div className="bg-white/5 border border-white/10 rounded-2xl p-6">
            <div className="flex items-center justify-between mb-6">
                <div className="flex items-center gap-3">
                    <Brain className="w-6 h-6 text-purple-400" />
                    <h3 className="text-xl font-bold text-white">Intelligence Dashboard</h3>
                </div>
                <button
                    onClick={handleRetrain}
                    disabled={training}
                    className="px-4 py-2 bg-purple-500/20 border border-purple-500/30 text-purple-400 rounded-lg hover:bg-purple-500/30 transition-colors flex items-center gap-2 disabled:opacity-50"
                >
                    <RefreshCw className={`w-4 h-4 ${training ? 'animate-spin' : ''}`} />
                    {training ? 'Training...' : 'Retrain Models'}
                </button>
            </div>

            {/* Stats Grid */}
            {stats && (
                <div className="grid grid-cols-2 md:grid-cols-4 gap-4 mb-6">
                    <div className="bg-white/5 rounded-lg p-4 border border-white/5">
                        <p className="text-gray-400 text-sm">Total Users</p>
                        <p className="text-2xl font-bold text-white">{stats.totalUsers}</p>
                    </div>
                    <div className="bg-white/5 rounded-lg p-4 border border-white/5">
                        <p className="text-gray-400 text-sm">Total Proposals</p>
                        <p className="text-2xl font-bold text-white">{stats.totalProposals}</p>
                    </div>
                    <div className="bg-white/5 rounded-lg p-4 border border-white/5">
                        <p className="text-gray-400 text-sm">Total Votes</p>
                        <p className="text-2xl font-bold text-white">{stats.totalVotes}</p>
                    </div>
                    <div className="bg-white/5 rounded-lg p-4 border border-white/5">
                        <p className="text-gray-400 text-sm flex items-center gap-1">
                            Potential Colluders
                            {stats.potentialColluders > 0 ?
                                <AlertTriangle className="w-3 h-3 text-red-400" /> :
                                <CheckCircle className="w-3 h-3 text-green-400" />
                            }
                        </p>
                        <p className={`text-2xl font-bold ${stats.potentialColluders > 0 ? 'text-red-400' : 'text-green-400'}`}>
                            {stats.potentialColluders}
                        </p>
                    </div>
                </div>
            )}

            {/* ML Classifications */}
            <div>
                <h4 className="text-lg font-semibold text-white mb-4 flex items-center gap-2">
                    <BarChart3 className="w-5 h-5 text-blue-400" />
                    ML Priority Classifications
                </h4>

                {classifications.length === 0 ? (
                    <p className="text-gray-500 text-center py-6">No proposals to classify yet</p>
                ) : (
                    <div className="space-y-3">
                        {classifications.map((item) => (
                            <motion.div
                                key={item.proposal.id}
                                layout
                                className="bg-white/5 rounded-lg p-4 border border-white/5"
                            >
                                <div className="flex items-center justify-between mb-3">
                                    <div>
                                        <h5 className="font-medium text-white">{item.proposal.title}</h5>
                                        <p className="text-sm text-gray-500">{item.proposal.voteCount} votes</p>
                                    </div>
                                    <span className={`px-3 py-1 rounded-full text-sm font-medium ${getPriorityColor(item.classification.ensemble.label)}`}>
                                        {item.classification.ensemble.label.replace('_', ' ')}
                                    </span>
                                </div>

                                <div className="grid grid-cols-3 gap-2 text-xs">
                                    <div className="bg-black/30 rounded p-2">
                                        <p className="text-gray-500 mb-1">Naive Bayes</p>
                                        <p className={getPriorityColor(item.classification.naiveBayes.label).split(' ')[0]}>
                                            {(item.classification.naiveBayes.confidence * 100).toFixed(1)}%
                                        </p>
                                    </div>
                                    <div className="bg-black/30 rounded p-2">
                                        <p className="text-gray-500 mb-1">Random Forest</p>
                                        <p className={getPriorityColor(item.classification.randomForest.label).split(' ')[0]}>
                                            {(item.classification.randomForest.confidence * 100).toFixed(1)}%
                                        </p>
                                    </div>
                                    <div className="bg-black/30 rounded p-2">
                                        <p className="text-gray-500 mb-1">Ensemble</p>
                                        <p className={getPriorityColor(item.classification.ensemble.label).split(' ')[0]}>
                                            {(item.classification.ensemble.confidence * 100).toFixed(1)}%
                                        </p>
                                    </div>
                                </div>
                            </motion.div>
                        ))}
                    </div>
                )}
            </div>
        </div>
    );
}
