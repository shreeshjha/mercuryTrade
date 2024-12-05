

export interface AssetBalance {
    symbol: string;
    available: number;
    locked: number;
    total: number;
    valueInUSD: number;
  }
  
  export interface PortfolioStats {
    totalValueUSD: number;
    dailyProfitLoss: number;
    dailyProfitLossPercentage: number;
    totalProfitLoss: number;
    totalProfitLossPercentage: number;
  }
  
  export interface TransactionHistory {
    id: string;
    type: 'deposit' | 'withdrawal' | 'trade';
    symbol: string;
    amount: number;
    status: 'completed' | 'pending' | 'failed';
    timestamp: Date;
    fee?: number;
  }