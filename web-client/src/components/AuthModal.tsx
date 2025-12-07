import { useState, useEffect } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { X, LogIn, UserPlus, AlertCircle, Calculator } from 'lucide-react';
import { useAuth } from '../context/AuthContext';

interface AuthModalProps {
    isOpen: boolean;
    onClose: () => void;
    initialMode?: 'login' | 'register';
}

export function AuthModal({ isOpen, onClose, initialMode = 'login' }: AuthModalProps) {
    const [mode, setMode] = useState<'login' | 'register'>(initialMode);
    const [username, setUsername] = useState('');
    const [password, setPassword] = useState('');
    const [error, setError] = useState('');
    const [loading, setLoading] = useState(false);
    const { login, register, registerWithCaptcha } = useAuth();

    // CAPTCHA state
    const [captcha, setCaptcha] = useState<{ id: string; question: string } | null>(null);
    const [captchaAnswer, setCaptchaAnswer] = useState('');

    useEffect(() => {
        setMode(initialMode);
    }, [initialMode]);

    const handleSubmit = async (e: React.FormEvent) => {
        e.preventDefault();
        setError('');
        setLoading(true);

        if (mode === 'login') {
            const result = await login(username, password);
            if (result.success) {
                onClose();
                resetForm();
            } else {
                setError(result.error || 'Login failed');
            }
        } else {
            // Registration
            if (captcha) {
                // Submit with CAPTCHA
                const result = await registerWithCaptcha(username, password, captcha.id, captchaAnswer);
                if (result.success) {
                    onClose();
                    resetForm();
                } else if (result.captcha) {
                    setCaptcha(result.captcha);
                    setCaptchaAnswer('');
                    setError(result.error || 'Incorrect answer');
                } else {
                    setError(result.error || 'Registration failed');
                }
            } else {
                // Initial registration attempt - will get CAPTCHA
                const result = await register(username, password);
                if (result.success) {
                    onClose();
                    resetForm();
                } else if (result.captcha) {
                    setCaptcha(result.captcha);
                    setError('');
                } else {
                    setError(result.error || 'Registration failed');
                }
            }
        }
        setLoading(false);
    };

    const resetForm = () => {
        setUsername('');
        setPassword('');
        setCaptcha(null);
        setCaptchaAnswer('');
        setError('');
    };

    const switchMode = (newMode: 'login' | 'register') => {
        setMode(newMode);
        setCaptcha(null);
        setCaptchaAnswer('');
        setError('');
    };

    return (
        <AnimatePresence>
            {isOpen && (
                <div className="fixed inset-0 z-[100] flex items-center justify-center p-4">
                    <motion.div
                        initial={{ opacity: 0 }} animate={{ opacity: 1 }} exit={{ opacity: 0 }}
                        className="absolute inset-0 bg-black/80 backdrop-blur-sm"
                        onClick={onClose}
                    />
                    <motion.div
                        initial={{ scale: 0.95, opacity: 0 }} animate={{ scale: 1, opacity: 1 }} exit={{ scale: 0.95, opacity: 0 }}
                        className="relative w-full max-w-md bg-[#0a0a0a] border border-white/10 rounded-2xl p-8 shadow-2xl"
                    >
                        <button onClick={onClose} className="absolute top-4 right-4 text-gray-400 hover:text-white">
                            <X className="w-5 h-5" />
                        </button>

                        <div className="flex items-center gap-3 mb-6">
                            {mode === 'login' ? <LogIn className="w-6 h-6 text-primary" /> : <UserPlus className="w-6 h-6 text-primary" />}
                            <h3 className="text-2xl font-bold text-white">
                                {mode === 'login' ? 'Sign In' : 'Create Account'}
                            </h3>
                        </div>

                        {error && (
                            <div className="mb-4 p-3 bg-red-500/10 border border-red-500/20 rounded-lg flex items-center gap-2 text-red-400 text-sm">
                                <AlertCircle className="w-4 h-4 flex-shrink-0" />
                                {error}
                            </div>
                        )}

                        <form onSubmit={handleSubmit} className="space-y-4">
                            <div>
                                <label className="block text-sm font-medium text-gray-400 mb-1">Username</label>
                                <input
                                    type="text" required
                                    value={username} onChange={e => setUsername(e.target.value)}
                                    className="w-full bg-white/5 border border-white/10 rounded-lg px-4 py-3 text-white focus:border-primary focus:outline-none"
                                    placeholder="Enter username"
                                    disabled={!!captcha}
                                />
                            </div>
                            <div>
                                <label className="block text-sm font-medium text-gray-400 mb-1">Password</label>
                                <input
                                    type="password" required minLength={6}
                                    value={password} onChange={e => setPassword(e.target.value)}
                                    className="w-full bg-white/5 border border-white/10 rounded-lg px-4 py-3 text-white focus:border-primary focus:outline-none"
                                    placeholder="Enter password (min 6 chars)"
                                    disabled={!!captcha}
                                />
                            </div>

                            {/* CAPTCHA Challenge */}
                            {captcha && (
                                <motion.div
                                    initial={{ opacity: 0, height: 0 }}
                                    animate={{ opacity: 1, height: 'auto' }}
                                    className="bg-purple-500/10 border border-purple-500/30 rounded-lg p-4"
                                >
                                    <div className="flex items-center gap-2 mb-3">
                                        <Calculator className="w-5 h-5 text-purple-400" />
                                        <span className="text-sm font-medium text-purple-300">Human Verification</span>
                                    </div>
                                    <p className="text-lg text-white mb-3 font-mono bg-black/30 p-3 rounded text-center">
                                        {captcha.question}
                                    </p>
                                    <input
                                        type="number"
                                        required
                                        value={captchaAnswer}
                                        onChange={e => setCaptchaAnswer(e.target.value)}
                                        className="w-full bg-white/5 border border-white/10 rounded-lg px-4 py-3 text-white focus:border-purple-400 focus:outline-none text-center text-lg"
                                        placeholder="Your answer"
                                        autoFocus
                                    />
                                </motion.div>
                            )}

                            <button
                                type="submit"
                                disabled={loading}
                                className="w-full bg-primary text-white font-medium py-3 rounded-lg hover:bg-blue-600 transition-colors disabled:opacity-50"
                            >
                                {loading ? 'Loading...' : captcha ? 'Verify & Register' : mode === 'login' ? 'Sign In' : 'Continue'}
                            </button>
                        </form>

                        <div className="mt-6 text-center text-sm text-gray-400">
                            {mode === 'login' ? (
                                <>
                                    Don't have an account?{' '}
                                    <button onClick={() => switchMode('register')} className="text-primary hover:underline">
                                        Sign up
                                    </button>
                                </>
                            ) : (
                                <>
                                    Already have an account?{' '}
                                    <button onClick={() => switchMode('login')} className="text-primary hover:underline">
                                        Sign in
                                    </button>
                                </>
                            )}
                        </div>
                    </motion.div>
                </div>
            )}
        </AnimatePresence>
    );
}
