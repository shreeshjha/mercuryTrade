import React from 'react';
import { useTradeHistory } from '../../contexts/TradingContext';
import { ArrowUpIcon, ArrowDownIcon } from '@heroicons/react/24/outline';

const TradeRow: React.FC<{ trade: any }> = ({ trade }) => {
  const isBuy = trade.side === 'buy';
  const Icon = isBuy ? ArrowUpIcon : ArrowDownIcon;

  return (
    <tr>
      <td className="px-4 py-2 whitespace-nowrap">
        <div className="flex items-center">
          <Icon 
            className={`h-4 w-4 ${isBuy ? 'text-green-500' : 'text-red-500'} mr-2`} 
          />
          <span className={isBuy ? 'text-green-600' : 'text-red-600'}>
            ${trade.price.toLocaleString()}
          </span>
        </div>
      </td>
      <td className="px-4 py-2 whitespace-nowrap text-gray-600 text-right">
        {trade.quantity.toFixed(6)}
      </td>
      <td className="px-4 py-2 whitespace-nowrap text-gray-500 text-right">
        ${(trade.price * trade.quantity).toLocaleString()}
      </td>
      <td className="px-4 py-2 whitespace-nowrap text-gray-500 text-right">
        {new Date(trade.timestamp).toLocaleTimeString()}
      </td>
    </tr>
  );
};

export default function TradeHistory() {
  const { trades, loading, error } = useTradeHistory();

  if (loading) {
    return (
      <div className="flex justify-center items-center h-full py-8">
        <div className="text-gray-500">Loading trades...</div>
      </div>
    );
  }

  if (error) {
    return (
      <div className="flex justify-center items-center h-full py-8">
        <div className="text-red-500">Error loading trades</div>
      </div>
    );
  }

  const calculateStats = () => {
    if (!trades.length) return { volume: 0, avgPrice: 0, highPrice: 0, lowPrice: 0 };

    const volume = trades.reduce((sum, trade) => sum + (trade.price * trade.quantity), 0);
    const prices = trades.map(t => t.price);
    const avgPrice = prices.reduce((a, b) => a + b, 0) / prices.length;
    const highPrice = Math.max(...prices);
    const lowPrice = Math.min(...prices);

    return { volume, avgPrice, highPrice, lowPrice };
  };

  const stats = calculateStats();

  return (
    <div className="space-y-4">
      {/* Stats Grid */}
      <div className="grid grid-cols-2 gap-4 mb-4">
        <div className="bg-gray-50 rounded-lg p-3">
          <div className="text-sm text-gray-500">24h Volume</div>
          <div className="text-lg font-medium">${stats.volume.toLocaleString()}</div>
        </div>
        <div className="bg-gray-50 rounded-lg p-3">
          <div className="text-sm text-gray-500">24h Avg Price</div>
          <div className="text-lg font-medium">${stats.avgPrice.toLocaleString()}</div>
        </div>
        <div className="bg-gray-50 rounded-lg p-3">
          <div className="text-sm text-gray-500">24h High</div>
          <div className="text-lg font-medium text-green-600">
            ${stats.highPrice.toLocaleString()}
          </div>
        </div>
        <div className="bg-gray-50 rounded-lg p-3">
          <div className="text-sm text-gray-500">24h Low</div>
          <div className="text-lg font-medium text-red-600">
            ${stats.lowPrice.toLocaleString()}
          </div>
        </div>
      </div>

      {/* Trades Table */}
      <div className="overflow-hidden">
        <table className="min-w-full divide-y divide-gray-200">
          <thead>
            <tr>
              <th className="px-4 py-2 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">
                Price
              </th>
              <th className="px-4 py-2 text-right text-xs font-medium text-gray-500 uppercase tracking-wider">
                Size
              </th>
              <th className="px-4 py-2 text-right text-xs font-medium text-gray-500 uppercase tracking-wider">
                Total
              </th>
              <th className="px-4 py-2 text-right text-xs font-medium text-gray-500 uppercase tracking-wider">
                Time
              </th>
            </tr>
          </thead>
          <tbody className="divide-y divide-gray-200">
            {trades.slice(0, 50).map((trade) => (
              <TradeRow key={trade.id} trade={trade} />
            ))}
          </tbody>
        </table>

        {trades.length === 0 && (
          <div className="text-center py-8 text-gray-500">
            No trades yet
          </div>
        )}
      </div>
    </div>
  );
}