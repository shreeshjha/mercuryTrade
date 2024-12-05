import React, { createContext, useContext, useState, useEffect } from 'react';
import { Order, MarketData, Trade, OrderBookSnapshot } from '../types/trading';

interface TradingContextType {
  // Market Data
  marketData: MarketData | null;
  selectedSymbol: string;
  setSelectedSymbol: (symbol: string) => void;
  availableSymbols: string[];
  
  // Orders
  activeOrders: Order[];
  submitOrder: (order: Omit<Order, 'id' | 'timestamp' | 'status'>) => Promise<void>;
  cancelOrder: (orderId: string) => Promise<void>;
  modifyOrder: (orderId: string, updates: Partial<Order>) => Promise<void>;
  
  // Trades
  trades: Trade[];
  
  // Order Book
  orderBookSnapshot: OrderBookSnapshot | null;
  
  // UI State
  loading: boolean;
  error: string | null;
}

const TradingContext = createContext<TradingContextType | undefined>(undefined);

export function TradingProvider({ children }: { children: React.ReactNode }) {
  // Market Data State
  const [marketData, setMarketData] = useState<MarketData | null>(null);
  const [selectedSymbol, setSelectedSymbol] = useState('BTC-USD');
  const [availableSymbols] = useState(['BTC-USD', 'ETH-USD', 'SOL-USD']);

  // Orders State
  const [activeOrders, setActiveOrders] = useState<Order[]>([]);
  
  // Trades State
  const [trades, setTrades] = useState<Trade[]>([]);
  
  // Order Book State
  const [orderBookSnapshot, setOrderBookSnapshot] = useState<OrderBookSnapshot | null>(null);
  
  // UI State
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  // WebSocket setup
  useEffect(() => {
    const ws = new WebSocket('wss://your-trading-api/ws');

    ws.onmessage = (event) => {
      const data = JSON.parse(event.data);
      
      switch (data.type) {
        case 'MARKET_DATA':
          if (data.symbol === selectedSymbol) {
            setMarketData(prevData => ({
              ...prevData,
              ...data.payload
            }));
          }
          break;
          
        case 'TRADE':
          if (data.symbol === selectedSymbol) {
            setTrades(prev => [data.payload, ...prev].slice(0, 100));
          }
          break;
          
        case 'ORDER_BOOK':
          if (data.symbol === selectedSymbol) {
            setOrderBookSnapshot(data.payload);
          }
          break;
          
        case 'ORDER_UPDATE':
          setActiveOrders(prevOrders => {
            const updatedOrders = prevOrders.filter(o => o.id !== data.payload.id);
            if (data.payload.status !== 'cancelled' && data.payload.status !== 'filled') {
              updatedOrders.push(data.payload);
            }
            return updatedOrders;
          });
          break;
      }
    };

    return () => {
      ws.close();
    };
  }, [selectedSymbol]);

  // Initial data fetch
  useEffect(() => {
    const fetchInitialData = async () => {
      setLoading(true);
      setError(null);
      
      try {
        // Simulated data for testing
        setMarketData({
          symbol: selectedSymbol,
          bid: 50000,
          ask: 50100,
          last: 50050,
          volume: 100,
          timestamp: new Date(),
          priceHistory: [
            { timestamp: new Date(Date.now() - 3600000), price: 49900 },
            { timestamp: new Date(Date.now() - 1800000), price: 50000 },
            { timestamp: new Date(), price: 50050 }
          ],
          orderBookSnapshot: {
            bids: [[49900, 1.5], [49800, 2.0], [49700, 1.0]],
            asks: [[50100, 1.0], [50200, 2.0], [50300, 1.5]],
            timestamp: new Date()
          }
        });

        setTrades([
          {
            id: '1',
            symbol: selectedSymbol,
            price: 50000,
            quantity: 1,
            side: 'buy',
            timestamp: new Date()
          }
        ]);

        setOrderBookSnapshot({
          bids: [[49900, 1.5], [49800, 2.0], [49700, 1.0]],
          asks: [[50100, 1.0], [50200, 2.0], [50300, 1.5]],
          timestamp: new Date()
        });

      } catch (err) {
        setError('Failed to fetch trading data');
        console.error('Error fetching data:', err);
      } finally {
        setLoading(false);
      }
    };

    fetchInitialData();
  }, [selectedSymbol]);

  const submitOrder = async (order: Omit<Order, 'id' | 'timestamp' | 'status'>) => {
    try {
      // Simulate order submission
      const newOrder: Order = {
        ...order,
        id: Math.random().toString(36).substring(7),
        timestamp: new Date(),
        status: 'pending'
      };
      
      setActiveOrders(prev => [...prev, newOrder]);
      
      // In real implementation, send to server
      // const response = await api.submitOrder(order);
      // setActiveOrders(prev => [...prev, response]);
    } catch (err) {
      setError('Failed to submit order');
      throw err;
    }
  };

  const cancelOrder = async (orderId: string) => {
    try {
      setActiveOrders(prev => prev.filter(order => order.id !== orderId));
      // In real implementation, send to server
      // await api.cancelOrder(orderId);
    } catch (err) {
      setError('Failed to cancel order');
      throw err;
    }
  };

  const modifyOrder = async (orderId: string, updates: Partial<Order>) => {
    try {
      setActiveOrders(prev =>
        prev.map(order =>
          order.id === orderId ? { ...order, ...updates } : order
        )
      );
      // In real implementation, send to server
      // await api.modifyOrder(orderId, updates);
    } catch (err) {
      setError('Failed to modify order');
      throw err;
    }
  };

  const value: TradingContextType = {
    marketData,
    selectedSymbol,
    setSelectedSymbol,
    availableSymbols,
    activeOrders,
    submitOrder,
    cancelOrder,
    modifyOrder,
    trades,
    orderBookSnapshot,
    loading,
    error
  };

  return (
    <TradingContext.Provider value={value}>
      {children}
    </TradingContext.Provider>
  );
}

export function useTradingContext() {
  const context = useContext(TradingContext);
  if (context === undefined) {
    throw new Error('useTradingContext must be used within a TradingProvider');
  }
  return context;
}

// Custom hooks for specific trading functionality
export function useMarketData() {
  const context = useTradingContext();
  return {
    marketData: context.marketData,
    selectedSymbol: context.selectedSymbol,
    setSelectedSymbol: context.setSelectedSymbol,
    availableSymbols: context.availableSymbols,
    loading: context.loading,
    error: context.error
  };
}

export function useOrderBook() {
  const context = useTradingContext();
  return {
    orderBookSnapshot: context.orderBookSnapshot,
    loading: context.loading,
    error: context.error
  };
}

export function useTradeHistory() {
  const context = useTradingContext();
  return {
    trades: context.trades,
    loading: context.loading,
    error: context.error
  };
}

export function useOrderManagement() {
  const context = useTradingContext();
  return {
    activeOrders: context.activeOrders,
    submitOrder: context.submitOrder,
    cancelOrder: context.cancelOrder,
    modifyOrder: context.modifyOrder,
    loading: context.loading,
    error: context.error
  };
}