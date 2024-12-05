export interface Position {
    symbol: string;
    size: number;
    entryPrice: number;
    markPrice: number;
    liquidationPrice?: number;
    margin: number;
    unrealizedPnL: number;
    realizedPnL: number;
    pnlPercentage: number;
    side: 'long' | 'short';
  }
  
  export interface PositionUpdate {
    symbol: string;
    markPrice: number;
    timestamp: Date;
  }