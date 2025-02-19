import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom';
import { QueryClient, QueryClientProvider } from '@tanstack/react-query';
import { AuthProvider, useAuth } from './contexts/AuthContext';
import { TradingProvider } from './contexts/TradingContext';
import MainLayout from './components/Layout/MainLayout';
import Dashboard from './pages/Dashboard';
import Trading from './pages/Trading';
import Login from './pages/Login';
import Register from './pages/Register';
import Profile from './pages/Profile';
import Portfolio from './pages/Portfolio';
import { PortfolioProvider } from './contexts/PortfolioContext';
import Positions from './pages/Positions';
import { PositionsProvider } from './contexts/PositionsContext';

const queryClient = new QueryClient();

function ProtectedRoute({ children }: { children: React.ReactNode }) {
  const { isAuthenticated, loading } = useAuth();

  console.log('ProtectedRoute:', { isAuthenticated, loading }); // Debug log

  if (loading) {
    return (
      <div className="flex items-center justify-center min-h-screen">
        <div className="text-lg">Loading...</div>
      </div>
    );
  }

  if (!isAuthenticated) {
    return <Navigate to="/portfolio" />; // Changed from /trading to /login
  }

  return <>{children}</>;
}

function App() {
  return (
    <QueryClientProvider client={queryClient}>
      <AuthProvider>
        <TradingProvider>
          <PortfolioProvider>
            <PositionsProvider>
            <Router>
            <Routes>
              {/* Public routes */}
              <Route path="/login" element={<Login />} />
              <Route path="/register" element={<Register />} />

              {/* Protected routes */}
              <Route
                path="/"
                element={
                  <ProtectedRoute>
                    <MainLayout>
                      <Dashboard />
                    </MainLayout>
                  </ProtectedRoute>
                }
              />
              <Route
                path="/trading"
                element={
                  <MainLayout>
                    <Trading />
                  </MainLayout>
                }
              />
              <Route
                path="/profile"
                element={
                  
                    <MainLayout>
                      <Profile />
                    </MainLayout>
                  
                }
              />
              <Route
                  path="/portfolio"
                  element={
                    
                      <MainLayout>
                        <Portfolio />
                      </MainLayout>
                    
                }
              />
              <Route 
                path="/positions" element={
                    <MainLayout>
                      <Positions />
                    </MainLayout>
                    
                  }
                />

              {/* Catch-all redirect to dashboard */}
              <Route path="*" element={<Navigate to="/" replace />} />
            </Routes>
          </Router>
            </PositionsProvider>
          
          </PortfolioProvider>
          
        </TradingProvider>
      </AuthProvider>
    </QueryClientProvider>
  );
}

export default App;