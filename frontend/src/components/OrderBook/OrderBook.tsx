import React, { useMemo } from 'react';
import { useOrderBook } from '../../contexts/TradingContext';

interface OrderBookLevel {
  price: number;
  size: number;
  total: number;
  percentage: number;
}

const OrderBookRow: React.FC<{
  level: OrderBookLevel;
  side: 'bid' | 'ask';
}> = ({ level, side }) => {
  const bgStyle = {
    background: `linear-gradient(${side === 'bid' ? 'to left' : 'to right'}, 
      ${side === 'bid' ? 'rgba(16, 185, 129, 0.1)' : 'rgba(239, 68, 68, 0.1)'} ${level.percentage}%, 
      transparent ${level.percentage}%)`
  };

  return (
    <div 
      className="grid grid-cols-3 text-sm py-1 hover:bg-gray-50"
      style={bgStyle}
    >
      <span className={`${side === 'bid' ? 'text-green-600' : 'text-red-600'} font-mono`}>
        {level.price.toLocaleString(undefined, { minimumFractionDigits: 2 })}
      </span>
      <span className="text-right font-mono">
        {level.size.toLocaleString(undefined, { minimumFractionDigits: 4 })}
      </span>
      <span className="text-right font-mono">
        {level.total.toLocaleString(undefined, { minimumFractionDigits: 4 })}
      </span>
    </div>
  );
};

export default function OrderBook() {
  const { orderBookSnapshot, loading, error } = useOrderBook();

  const { bids, asks } = useMemo(() => {
    if (!orderBookSnapshot) {
      return { bids: [], asks: [] };
    }

    // Process bids
    let runningTotal = 0;
    const maxTotal = Math.max(
      orderBookSnapshot.bids.reduce((acc, [_, size]) => acc + size, 0),
      orderBookSnapshot.asks.reduce((acc, [_, size]) => acc + size, 0)
    );

    const processedBids: OrderBookLevel[] = orderBookSnapshot.bids
      .map(([price, size]) => {
        runningTotal += size;
        return {
          price,
          size,
          total: runningTotal,
          percentage: (runningTotal / maxTotal) * 100
        };
      });

    // Process asks (reverse order)
    runningTotal = 0;
    const processedAsks: OrderBookLevel[] = [...orderBookSnapshot.asks]
      .reverse()
      .map(([price, size]) => {
        runningTotal += size;
        return {
          price,
          size,
          total: runningTotal,
          percentage: (runningTotal / maxTotal) * 100
        };
      });

    return {
      bids: processedBids,
      asks: processedAsks
    };
  }, [orderBookSnapshot]);

  if (loading) {
    return (
      <div className="flex justify-center items-center h-96">
        <div className="text-gray-500">Loading order book...</div>
      </div>
    );
  }

  if (error) {
    return (
      <div className="flex justify-center items-center h-96">
        <div className="text-red-500">Error loading order book</div>
      </div>
    );
  }

  const spread = asks[asks.length - 1]?.price - bids[0]?.price;
  const spreadPercentage = spread ? (spread / asks[asks.length - 1]?.price) * 100 : 0;

  return (
    <div className="h-full flex flex-col">
      {/* Header */}
      <div className="grid grid-cols-3 text-xs text-gray-500 pb-2">
        <span>Price</span>
        <span className="text-right">Size</span>
        <span className="text-right">Total</span>
      </div>

      {/* Asks (Sells) */}
      <div className="flex-1 overflow-auto">
        {asks.map((level, i) => (
          <OrderBookRow key={`ask-${i}`} level={level} side="ask" />
        ))}
      </div>

      {/* Spread */}
      <div className="py-2 text-xs text-gray-500 border-y border-gray-200">
        <div className="flex justify-between">
          <span>Spread</span>
          <span>{spread.toFixed(2)} ({spreadPercentage.toFixed(2)}%)</span>
        </div>
      </div>

      {/* Bids (Buys) */}
      <div className="flex-1 overflow-auto">
        {bids.map((level, i) => (
          <OrderBookRow key={`bid-${i}`} level={level} side="bid" />
        ))}
      </div>
    </div>
  );
}