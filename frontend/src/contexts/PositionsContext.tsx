// src/contexts/PositionsContext.tsx
import React, { createContext, useContext, useState, useEffect } from 'react';
import { Position, PositionUpdate } from '../types/positions';

interface PositionsContextType {
  positions: Position[];
  totalPnL: number;
  dailyPnL: number;
  loading: boolean;
  error: string | null;
  closePosition: (symbol: string) => Promise<void>;
  updateLeverage: (symbol: string, leverage: number) => Promise<void>;
}

const PositionsContext = createContext<PositionsContextType | undefined>(undefined);

export function PositionsProvider({ children }: { children: React.ReactNode }) {
  const [positions, setPositions] = useState<Position[]>([]);
  const [totalPnL, setTotalPnL] = useState(0);
  const [dailyPnL, setDailyPnL] = useState(0);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    // Simulated initial positions
    setPositions([
      {
        symbol: 'BTC-USD',
        size: 1.5,
        entryPrice: 48000,
        markPrice: 50000,
        liquidationPrice: 40000,
        margin: 15000,
        unrealizedPnL: 3000,
        realizedPnL: 1500,
        pnlPercentage: 6.25,
        side: 'long'
      }
    ]);
    setLoading(false);
  }, []);

  useEffect(() => {
    // Calculate total PnL
    const totalUnrealized = positions.reduce((sum, pos) => sum + pos.unrealizedPnL, 0);
    const totalRealized = positions.reduce((sum, pos) => sum + pos.realizedPnL, 0);
    setTotalPnL(totalUnrealized + totalRealized);
  }, [positions]);

  const closePosition = async (symbol: string) => {
    try {
      // Simulate API call
      setPositions(prev => prev.filter(pos => pos.symbol !== symbol));
    } catch (err) {
      setError('Failed to close position');
    }
  };

  const updateLeverage = async (symbol: string, leverage: number) => {
    try {
      // Simulate API call
      setPositions(prev => 
        prev.map(pos => 
          pos.symbol === symbol 
            ? { ...pos, margin: pos.size * pos.markPrice / leverage }
            : pos
        )
      );
    } catch (err) {
      setError('Failed to update leverage');
    }
  };

  return (
    <PositionsContext.Provider value={{
      positions,
      totalPnL,
      dailyPnL,
      loading,
      error,
      closePosition,
      updateLeverage
    }}>
      {children}
    </PositionsContext.Provider>
  );
}

export function usePositions() {
  const context = useContext(PositionsContext);
  if (!context) {
    throw new Error('usePositions must be used within PositionsProvider');
  }
  return context;
}