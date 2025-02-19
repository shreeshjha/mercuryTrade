import React, { useState } from 'react';
import {
  AreaChart,
  Area,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer
} from 'recharts';
import { useMarketData } from '../../contexts/TradingContext';

const timeframes = [
  { label: '1H', value: '1h' },
  { label: '1D', value: '1d' },
  { label: '1W', value: '1w' },
  { label: '1M', value: '1m' }
] as const;

type Timeframe = typeof timeframes[number]['value'];

const CustomTooltip = ({ active, payload, label }: any) => {
  if (active && payload && payload.length) {
    return (
      <div className="bg-white p-3 border border-gray-200 rounded shadow-lg">
        <p className="text-sm font-medium text-gray-900">
          ${payload[0].value.toLocaleString()}
        </p>
        <p className="text-xs text-gray-500">
          {new Date(label).toLocaleString()}
        </p>
      </div>
    );
  }
  return null;
};

export default function PriceChart() {
  const { marketData, loading, error } = useMarketData();
  const [timeframe, setTimeframe] = useState<Timeframe>('1d');

  if (loading) {
    return (
      <div className="flex justify-center items-center h-96">
        <div className="text-gray-500">Loading chart data...</div>
      </div>
    );
  }

  if (error) {
    return (
      <div className="flex justify-center items-center h-96">
        <div className="text-red-500">Error loading chart data</div>
      </div>
    );
  }

  const data = marketData?.priceHistory?.map(point => ({
    timestamp: new Date(point.timestamp).getTime(),
    price: point.price
  })) || [];

  const formatXAxis = (timestamp: number) => {
    const date = new Date(timestamp);
    switch (timeframe) {
      case '1h':
        return date.toLocaleTimeString();
      case '1d':
        return `${date.getHours()}:${String(date.getMinutes()).padStart(2, '0')}`;
      default:
        return date.toLocaleDateString();
    }
  };

  return (
    <div className="h-full">
      {/* Timeframe Selector */}
      <div className="flex space-x-2 mb-4">
        {timeframes.map(({ label, value }) => (
          <button
            key={value}
            onClick={() => setTimeframe(value)}
            className={`px-4 py-2 text-sm font-medium rounded-lg ${
              timeframe === value
                ? 'bg-blue-600 text-white'
                : 'bg-gray-100 text-gray-600 hover:bg-gray-200'
            }`}
          >
            {label}
          </button>
        ))}
      </div>

      {/* Price Display */}
      {marketData && (
        <div className="mb-4">
          <div className="text-3xl font-bold text-gray-900">
            ${marketData.last.toLocaleString()}
          </div>
          <div className="text-sm text-gray-500">
            Current Price
          </div>
        </div>
      )}

      {/* Chart */}
      <div className="h-[400px]">
        <ResponsiveContainer width="100%" height="100%">
          <AreaChart
            data={data}
            margin={{
              top: 10,
              right: 30,
              left: 0,
              bottom: 0,
            }}
          >
            <CartesianGrid strokeDasharray="3 3" />
            <XAxis
              dataKey="timestamp"
              tickFormatter={formatXAxis}
              type="number"
              domain={['auto', 'auto']}
            />
            <YAxis
              domain={['auto', 'auto']}
              tickFormatter={(value) => `$${value.toLocaleString()}`}
            />
            <Tooltip content={<CustomTooltip />} />
            <Area
              type="monotone"
              dataKey="price"
              stroke="#2563eb"
              fill="#3b82f6"
              fillOpacity={0.1}
            />
          </AreaChart>
        </ResponsiveContainer>
      </div>
    </div>
  );
}