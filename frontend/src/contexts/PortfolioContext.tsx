
import React, { createContext, useContext, useState, useEffect } from 'react';
import { AssetBalance, PortfolioStats, TransactionHistory } from '../types/portfolio';

interface PortfolioContextType {
  balances: AssetBalance[];
  stats: PortfolioStats;
  transactions: TransactionHistory[];
  loading: boolean;
  error: string | null;
  refreshBalances: () => Promise<void>;
}

const PortfolioContext = createContext<PortfolioContextType | undefined>(undefined);

export function PortfolioProvider({ children }: { children: React.ReactNode }) {
  const [balances, setBalances] = useState<AssetBalance[]>([]);
  const [stats, setStats] = useState<PortfolioStats>({
    totalValueUSD: 0,
    dailyProfitLoss: 0,
    dailyProfitLossPercentage: 0,
    totalProfitLoss: 0,
    totalProfitLossPercentage: 0
  });
  const [transactions, setTransactions] = useState<TransactionHistory[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  const refreshBalances = async () => {
    try {
      setLoading(true);
      // In real implementation, fetch from API
      // Simulated data
      setBalances([
        {
          symbol: 'BTC',
          available: 1.5,
          locked: 0.5,
          total: 2.0,
          valueInUSD: 80000
        },
        {
          symbol: 'USD',
          available: 50000,
          locked: 10000,
          total: 60000,
          valueInUSD: 60000
        }
      ]);
      
      setStats({
        totalValueUSD: 140000,
        dailyProfitLoss: 5000,
        dailyProfitLossPercentage: 3.7,
        totalProfitLoss: 15000,
        totalProfitLossPercentage: 12.0
      });

      setTransactions([
        {
          id: '1',
          type: 'trade',
          symbol: 'BTC',
          amount: 0.5,
          status: 'completed',
          timestamp: new Date(),
          fee: 0.001
        }
      ]);
    } catch (err) {
      setError('Failed to fetch portfolio data');
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    refreshBalances();
  }, []);

  return (
    <PortfolioContext.Provider value={{
      balances,
      stats,
      transactions,
      loading,
      error,
      refreshBalances
    }}>
      {children}
    </PortfolioContext.Provider>
  );
}

export function usePortfolio() {
  const context = useContext(PortfolioContext);
  if (context === undefined) {
    throw new Error('usePortfolio must be used within a PortfolioProvider');
  }
  return context;
}