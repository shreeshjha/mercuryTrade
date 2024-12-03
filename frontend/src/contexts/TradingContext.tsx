import { createContext, useContext, useState, useEffect, ReactNode } from 'react';
import { Order, MarketData, Trade } from '../types/trading';
import { tradingApi } from '../services/api';
import { WebSocketService } from '../services/websocket';

interface TradingContextType {
  activeOrders: Order[];
  marketData: MarketData | null;
  trades: Trade[];
  loading: boolean;
  error: string | null;
  submitOrder: (order: Omit<Order, 'id' | 'timestamp' | 'status'>) => Promise<void>;
  cancelOrder: (orderId: string) => Promise<void>;
  selectedSymbol: string;
  setSelectedSymbol: (symbol: string) => void;
}

const TradingContext = createContext<TradingContextType | undefined>(undefined);

const wsService = new WebSocketService('ws://localhost:8080');

export function TradingProvider({ children }: { children: ReactNode }) {
  const [activeOrders, setActiveOrders] = useState<Order[]>([]);
  const [marketData, setMarketData] = useState<MarketData | null>(null);
  const [trades, setTrades] = useState<Trade[]>([]);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [selectedSymbol, setSelectedSymbol] = useState('BTC-USD');

  // Initialize WebSocket connection and subscriptions
  useEffect(() => {
    wsService.connect();

    // Subscribe to market data updates
    wsService.subscribe('marketData', (data: MarketData) => {
      if (data.symbol === selectedSymbol) {
        setMarketData(data);
      }
    });

    // Subscribe to trade updates
    wsService.subscribe('trade', (trade: Trade) => {
      if (trade.symbol === selectedSymbol) {
        setTrades(prev => [trade, ...prev].slice(0, 100)); // Keep last 100 trades
      }
    });

    // Subscribe to order updates
    wsService.subscribe('order', (order: Order) => {
      setActiveOrders(prev => {
        const filtered = prev.filter(o => o.id !== order.id);
        if (order.status === 'pending') {
          return [...filtered, order];
        }
        return filtered;
      });
    });

    return () => {
      wsService.unsubscribe('marketData', setMarketData);
      wsService.unsubscribe('trade', data => setTrades(prev => [data, ...prev]));
    };
  }, [selectedSymbol]);

  // Fetch initial data when symbol changes
  useEffect(() => {
    const fetchData = async () => {
      setLoading(true);
      setError(null);
      try {
        const [marketDataResponse, tradesResponse] = await Promise.all([
          tradingApi.getMarketData(selectedSymbol),
          tradingApi.getTradeHistory(selectedSymbol)
        ]);
        
        setMarketData(marketDataResponse);
        setTrades(tradesResponse);
      } catch (err) {
        setError(err instanceof Error ? err.message : 'An error occurred');
        console.error('Error fetching trading data:', err);
      } finally {
        setLoading(false);
      }
    };

    fetchData();
  }, [selectedSymbol]);

  const submitOrder = async (order: Omit<Order, 'id' | 'timestamp' | 'status'>) => {
    setError(null);
    try {
      const newOrder = await tradingApi.submitOrder(order);
      setActiveOrders(prev => [...prev, newOrder]);
      
      // Send order to WebSocket for real-time updates
      wsService.send({
        type: 'newOrder',
        payload: newOrder
      });
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to submit order');
      throw err;
    }
  };

  const cancelOrder = async (orderId: string) => {
    setError(null);
    try {
      await tradingApi.cancelOrder(orderId);
      setActiveOrders(prev => prev.filter(order => order.id !== orderId));
      
      // Send cancellation to WebSocket
      wsService.send({
        type: 'cancelOrder',
        payload: { orderId }
      });
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to cancel order');
      throw err;
    }
  };

  // Calculate some derived data
  const value: TradingContextType = {
    activeOrders,
    marketData,
    trades,
    loading,
    error,
    submitOrder,
    cancelOrder,
    selectedSymbol,
    setSelectedSymbol
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

// Custom hook for market data updates
export function useMarketData() {
  const context = useTradingContext();
  return {
    marketData: context.marketData,
    loading: context.loading,
    error: context.error
  };
}

// Custom hook for trade history
export function useTradeHistory() {
  const context = useTradingContext();
  return {
    trades: context.trades,
    loading: context.loading,
    error: context.error
  };
}

// Custom hook for order management
export function useOrderManagement() {
  const context = useTradingContext();
  return {
    activeOrders: context.activeOrders,
    submitOrder: context.submitOrder,
    cancelOrder: context.cancelOrder,
    loading: context.loading,
    error: context.error
  };
}