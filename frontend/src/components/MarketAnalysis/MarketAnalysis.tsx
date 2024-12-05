import React from 'react';
import MarketDepth from './MarketDepth';
import { useTradingContext } from '../../contexts/TradingContext';

const MarketAnalysis = () => {
  const { marketData } = useTradingContext();

  const getSpread = () => {
    if (!marketData?.ask || !marketData?.bid) return 0;
    return marketData.ask - marketData.bid;
  };

  const getSpreadPercentage = () => {
    if (!marketData?.ask || !marketData?.bid) return 0;
    return ((marketData.ask - marketData.bid) / marketData.bid) * 100;
  };

  return (
    <div className="space-y-6">
      <div className="bg-white shadow rounded-lg p-6">
        <h2 className="text-lg font-medium text-gray-900 mb-4">Market Analysis</h2>
        
        <div className="grid grid-cols-2 gap-4 mb-6">
          <div className="bg-gray-50 rounded-lg p-4">
            <p className="text-sm text-gray-500">Spread</p>
            <p className="text-lg font-semibold text-gray-900">
              ${getSpread().toFixed(2)}
            </p>
            <p className="text-xs text-gray-500">
              {getSpreadPercentage().toFixed(4)}%
            </p>
          </div>
          
          <div className="bg-gray-50 rounded-lg p-4">
            <p className="text-sm text-gray-500">24h Volume</p>
            <p className="text-lg font-semibold text-gray-900">
              ${marketData?.volume.toLocaleString() ?? 0}
            </p>
          </div>
        </div>

        <div className="space-y-4">
          <h3 className="text-md font-medium text-gray-900">Market Depth</h3>
          <MarketDepth />
        </div>
      </div>
    </div>
  );
};

export default MarketAnalysis;