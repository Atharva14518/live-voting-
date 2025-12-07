import { useState } from 'react';
import { motion } from 'framer-motion';
import { ArrowRight, BarChart2, Shield, Users, LogOut, User } from 'lucide-react';
import { Dashboard } from './components/Dashboard';
import { AuthModal } from './components/AuthModal';
import { IntelligenceDashboard } from './components/IntelligenceDashboard';
import { AuthProvider, useAuth } from './context/AuthContext';

function AppContent() {
  const { user, logout, loading } = useAuth();
  const [showAuthModal, setShowAuthModal] = useState(false);
  const [authMode, setAuthMode] = useState<'login' | 'register'>('login');

  const scrollToDashboard = () => {
    const element = document.getElementById('dashboard');
    if (element) {
      element.scrollIntoView({ behavior: 'smooth' });
    }
  };

  const handleAuthClick = (mode: 'login' | 'register') => {
    setAuthMode(mode);
    setShowAuthModal(true);
  };

  return (
    <div className="min-h-screen bg-background text-foreground overflow-x-hidden selection:bg-primary/30">
      {/* Navigation */}
      <nav className="fixed top-0 w-full z-50 glass border-b border-white/10">
        <div className="max-w-7xl mx-auto px-6 h-16 flex items-center justify-between">
          <div className="flex items-center gap-3">
            <img src="/logo.png" alt="CrowdDecision" className="w-10 h-10 rounded-lg" />
            <span className="font-bold text-xl tracking-tight">CrowdDecision</span>
          </div>
          <div className="hidden md:flex items-center gap-8 text-sm font-medium text-gray-400">
            <a href="#features" className="hover:text-white transition-colors">Features</a>
            <a href="#dashboard" onClick={(e) => { e.preventDefault(); scrollToDashboard(); }} className="hover:text-white transition-colors">Live Rankings</a>
            <a href="#intelligence" className="hover:text-white transition-colors">Intelligence</a>
          </div>
          <div className="flex items-center gap-4">
            {loading ? (
              <span className="text-gray-500">Loading...</span>
            ) : user ? (
              <div className="flex items-center gap-4">
                <div className="flex items-center gap-2 text-sm">
                  <User className="w-4 h-4 text-primary" />
                  <span className="text-white font-medium">{user.username}</span>
                </div>
                <button
                  onClick={logout}
                  className="text-sm font-medium text-gray-400 hover:text-white transition-colors flex items-center gap-1"
                >
                  <LogOut className="w-4 h-4" />
                  Logout
                </button>
              </div>
            ) : (
              <>
                <button
                  onClick={() => handleAuthClick('login')}
                  className="text-sm font-medium text-gray-400 hover:text-white transition-colors"
                >
                  Sign In
                </button>
                <button
                  onClick={() => handleAuthClick('register')}
                  className="bg-white text-black px-4 py-2 rounded-full text-sm font-medium hover:bg-gray-200 transition-colors"
                >
                  Get Started
                </button>
              </>
            )}
          </div>
        </div>
      </nav>

      {/* Hero Section */}
      <section className="relative pt-32 pb-20 px-6">
        <div className="absolute inset-0 bg-[radial-gradient(ellipse_at_top,_var(--tw-gradient-stops))] from-primary/20 via-background to-background pointer-events-none" />

        <div className="max-w-7xl mx-auto text-center relative z-10">
          <motion.div
            initial={{ opacity: 0, y: 20 }}
            animate={{ opacity: 1, y: 0 }}
            transition={{ duration: 0.5 }}
          >
            <div className="flex justify-center mb-8">
              <img src="/logo.png" alt="CrowdDecision" className="w-24 h-24 rounded-2xl shadow-2xl" />
            </div>
            <span className="inline-block py-1 px-3 rounded-full bg-white/5 border border-white/10 text-sm text-primary mb-6">
              v2.0 Now Available — AI-Powered
            </span>
            <h1 className="text-5xl md:text-7xl font-bold tracking-tight mb-8 bg-clip-text text-transparent bg-gradient-to-b from-white to-white/60">
              Decisions made <br />
              <span className="text-white">together, smarter.</span>
            </h1>
            <p className="text-xl text-gray-400 max-w-2xl mx-auto mb-10 leading-relaxed">
              A production-ready platform for collaborative decision making.
              Powered by <strong className="text-white">Naive Bayes</strong>, <strong className="text-white">Random Forest</strong>, and real-time analytics.
            </p>

            <div className="flex flex-col sm:flex-row items-center justify-center gap-4">
              <button onClick={scrollToDashboard} className="w-full sm:w-auto px-8 py-4 bg-white text-black rounded-full font-medium hover:bg-gray-200 transition-all flex items-center justify-center gap-2 group">
                Start Voting
                <ArrowRight className="w-4 h-4 group-hover:translate-x-1 transition-transform" />
              </button>
              <button onClick={() => document.getElementById('intelligence')?.scrollIntoView({ behavior: 'smooth' })} className="w-full sm:w-auto px-8 py-4 bg-white/5 border border-white/10 rounded-full font-medium hover:bg-white/10 transition-all text-white">
                View Intelligence
              </button>
            </div>
          </motion.div>
        </div>
      </section>

      {/* Features Grid */}
      <section id="features" className="py-24 px-6 bg-black/50">
        <div className="max-w-7xl mx-auto">
          <div className="grid md:grid-cols-3 gap-8">
            {[
              {
                icon: <BarChart2 className="w-6 h-6 text-blue-400" />,
                title: "Real-time Analytics",
                desc: "Live ranking updates using efficient max-heap data structures with O(log n) complexity."
              },
              {
                icon: <Shield className="w-6 h-6 text-purple-400" />,
                title: "Anti-Abuse System",
                desc: "Advanced bot detection, velocity analysis, and collusion prevention algorithms."
              },
              {
                icon: <Users className="w-6 h-6 text-green-400" />,
                title: "Crowd Intelligence",
                desc: "Ensemble ML models: Naive Bayes + Random Forest with meta-learning."
              }
            ].map((feature, i) => (
              <motion.div
                key={i}
                initial={{ opacity: 0, y: 20 }}
                whileInView={{ opacity: 1, y: 0 }}
                transition={{ delay: i * 0.1 }}
                className="p-6 rounded-2xl bg-white/5 border border-white/10 hover:bg-white/10 transition-colors"
              >
                <div className="w-12 h-12 rounded-lg bg-white/5 flex items-center justify-center mb-4">
                  {feature.icon}
                </div>
                <h3 className="text-xl font-semibold mb-2 text-white">{feature.title}</h3>
                <p className="text-gray-400 leading-relaxed">{feature.desc}</p>
              </motion.div>
            ))}
          </div>
        </div>
      </section>

      {/* Dashboard */}
      <Dashboard />

      {/* Intelligence Dashboard */}
      <section id="intelligence" className="py-16 px-6">
        <div className="max-w-7xl mx-auto">
          <IntelligenceDashboard />
        </div>
      </section>

      {/* Auth Modal */}
      <AuthModal
        isOpen={showAuthModal}
        onClose={() => setShowAuthModal(false)}
        initialMode={authMode}
      />
    </div>
  );
}

function App() {
  return (
    <AuthProvider>
      <AppContent />
    </AuthProvider>
  );
}

export default App;
