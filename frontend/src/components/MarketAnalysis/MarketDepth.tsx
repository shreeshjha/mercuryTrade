import React, { useMemo } from 'react';
import { AreaChart, Area, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';
import { useTradingContext } from '../../contexts/TradingContext';

interface DepthData {
  price: number;
  bids: number;
  asks: number;
  totalBids: number;
  totalAsks: number;
}

const MarketDepth = () => {
  const { marketData } = useTradingContext();

  const depthData = useMemo(() => {
    if (!marketData?.orderBookSnapshot) return [];

    const data: DepthData[] = [];
    let totalBids = 0;
    let totalAsks = 0;

    // Process bids (buy orders)
    marketData.orderBookSnapshot.bids.forEach(([price, size]) => {
      totalBids += size;
      data.push({
        price,
        bids: size,
        asks: 0,
        totalBids,
        totalAsks: 0
      });
    });

    // Process asks (sell orders)
    marketData.orderBookSnapshot.asks.forEach(([price, size]) => {
      totalAsks += size;
      data.push({
        price,
        bids: 0,
        asks: size,
        totalBids: 0,
        totalAsks
      });
    });

    // Sort by price
    return data.sort((a, b) => a.price - b.price);
  }, [marketData?.orderBookSnapshot]);

  const CustomTooltip = ({ active, payload }: any) => {
    if (active && payload && payload.length) {
      const data = payload[0].payload;
      return (
        <div className="bg-white p-4 shadow rounded border border-gray-200">
          <p className="text-sm font-medium text-gray-900">Price: ${data.price.toLocaleString()}</p>
          {data.totalBids > 0 && (
            <p className="text-sm text-green-600">Cumulative Bids: {data.totalBids.toFixed(4)}</p>
          )}
          {data.totalAsks > 0 && (
            <p className="text-sm text-red-600">Cumulative Asks: {data.totalAsks.toFixed(4)}</p>
          )}
        </div>
      );
    }
    return null;
  };

  return (
    <div className="h-96">
      <ResponsiveContainer width="100%" height="100%">
        <AreaChart
          data={depthData}
          margin={{
            top: 10,
            right: 30,
            left: 0,
            bottom: 0,
          }}
        >
          <CartesianGrid strokeDasharray="3 3" />
          <XAxis 
            dataKey="price" 
            type="number" 
            domain={['auto', 'auto']}
            tickFormatter={(value) => `$${value.toLocaleString()}`}
          />
          <YAxis />
          <Tooltip content={<CustomTooltip />} />
          <Area
            type="monotone"
            dataKey="totalBids"
            stackId="1"
            stroke="#059669"
            fill="#059669"
            fillOpacity={0.3}
          />
          <Area
            type="monotone"
            dataKey="totalAsks"
            stackId="2"
            stroke="#DC2626"
            fill="#DC2626"
            fillOpacity={0.3}
          />
        </AreaChart>
      </ResponsiveContainer>
    </div>
  );
};

export default MarketDepth;