// src/types/trading.ts

export interface Order {
  id: string;
  symbol: string;
  side: 'buy' | 'sell';
  type: 'market' | 'limit';
  price?: number;
  quantity: number;
  timestamp: Date;
  status: 'pending' | 'filled' | 'cancelled' | 'rejected';
}

export interface MarketData {
  symbol: string;
  bid: number;
  ask: number;
  last: number;
  volume: number;
  timestamp: Date;
  priceHistory?: Array<{
    timestamp: Date;
    price: number;
  }>;
  orderBookSnapshot?: OrderBookSnapshot;
}

export interface Trade {
  id: string;
  symbol: string;
  price: number;
  quantity: number;
  side: 'buy' | 'sell';
  timestamp: Date;
}

export interface OrderBookSnapshot {
  bids: Array<[number, number]>; // Array of [price, size] tuples
  asks: Array<[number, number]>; // Array of [price, size] tuples
  timestamp: Date;
}

export interface MarketDepthLevel {
  price: number;
  size: number;
  total: number;
  side: 'bid' | 'ask';
}