export interface Order {
    id: string;
    symbol: string;
    side: 'buy' | 'sell';
    type: 'market' | 'limit';
    price?: number;
    quantity: number;
    timestamp: Date;
    status: 'pending' | 'filled' | 'cancelled';
  }
  
  export interface MarketData {
    symbol: string;
    bid: number;
    ask: number;
    last: number;
    volume: number;
    timestamp: Date;
  }
  
  export interface Trade {
    id: string;
    symbol: string;
    price: number;
    quantity: number;
    side: 'buy' | 'sell';
    timestamp: Date;
  }