import { useState, useEffect, createContext, useContext, type ReactNode } from 'react';

const API_URL = import.meta.env.VITE_API_URL || 'http://localhost:3000';

interface User {
    id: string;
    username: string;
    trustScore: number;
}

interface AuthContextType {
    user: User | null;
    token: string | null;
    loading: boolean;
    login: (username: string, password: string) => Promise<{ success: boolean; error?: string }>;
    register: (username: string, password: string) => Promise<{ success: boolean; error?: string; captcha?: { id: string; question: string } }>;
    registerWithCaptcha: (username: string, password: string, captchaId: string, captchaAnswer: string) => Promise<{ success: boolean; error?: string; captcha?: { id: string; question: string } }>;
    logout: () => void;
}

const AuthContext = createContext<AuthContextType | null>(null);

export function AuthProvider({ children }: { children: ReactNode }) {
    const [user, setUser] = useState<User | null>(null);
    const [token, setToken] = useState<string | null>(() => localStorage.getItem('token'));
    const [loading, setLoading] = useState(true);

    useEffect(() => {
        if (token) {
            fetch(`${API_URL}/api/auth/me`, {
                headers: { Authorization: `Bearer ${token}` }
            })
                .then(res => res.json())
                .then(data => {
                    if (data.user) setUser(data.user);
                    else {
                        localStorage.removeItem('token');
                        setToken(null);
                    }
                })
                .catch(() => {
                    localStorage.removeItem('token');
                    setToken(null);
                })
                .finally(() => setLoading(false));
        } else {
            setLoading(false);
        }
    }, [token]);

    const login = async (username: string, password: string) => {
        try {
            const res = await fetch(`${API_URL}/api/auth/login`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ username, password })
            });
            const data = await res.json();
            if (data.error) return { success: false, error: data.error };

            setUser(data.user);
            setToken(data.token);
            localStorage.setItem('token', data.token);
            return { success: true };
        } catch {
            return { success: false, error: 'Network error' };
        }
    };

    const register = async (username: string, password: string) => {
        try {
            const res = await fetch(`${API_URL}/api/auth/register`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ username, password })
            });
            const data = await res.json();

            if (data.captcha) {
                return { success: false, error: data.error, captcha: data.captcha };
            }

            if (data.error) return { success: false, error: data.error };

            setUser(data.user);
            setToken(data.token);
            localStorage.setItem('token', data.token);
            return { success: true };
        } catch {
            return { success: false, error: 'Network error' };
        }
    };

    const registerWithCaptcha = async (username: string, password: string, captchaId: string, captchaAnswer: string) => {
        try {
            const res = await fetch(`${API_URL}/api/auth/register`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ username, password, captchaId, captchaAnswer })
            });
            const data = await res.json();

            if (data.captcha) {
                return { success: false, error: data.error, captcha: data.captcha };
            }

            if (data.error) return { success: false, error: data.error };

            setUser(data.user);
            setToken(data.token);
            localStorage.setItem('token', data.token);
            return { success: true };
        } catch {
            return { success: false, error: 'Network error' };
        }
    };

    const logout = () => {
        setUser(null);
        setToken(null);
        localStorage.removeItem('token');
    };

    return (
        <AuthContext.Provider value={{ user, token, loading, login, register, registerWithCaptcha, logout }}>
            {children}
        </AuthContext.Provider>
    );
}

export function useAuth() {
    const context = useContext(AuthContext);
    if (!context) throw new Error('useAuth must be used within AuthProvider');
    return context;
}
