import React from 'react';

import { ArrowUpIcon, ArrowDownIcon } from '@heroicons/react/24/outline';
import { usePortfolio } from '../../contexts/PortfolioContext';

export function PortfolioOverview() {
  const { stats, loading, error } = usePortfolio();

  if (loading) return <div className="text-gray-500">Loading portfolio...</div>;
  if (error) return <div className="text-red-500">{error}</div>;

  return (
    <div className="space-y-6">
      <div className="grid grid-cols-2 gap-4">
        <div className="bg-white rounded-lg p-4 shadow">
          <div className="text-sm text-gray-500">Portfolio Value</div>
          <div className="text-2xl font-bold">${stats.totalValueUSD.toLocaleString()}</div>
        </div>
        
        <div className="bg-white rounded-lg p-4 shadow">
          <div className="text-sm text-gray-500">24h Profit/Loss</div>
          <div className="flex items-center">
            <div className={`text-2xl font-bold ${stats.dailyProfitLoss >= 0 ? 'text-green-600' : 'text-red-600'}`}>
              ${Math.abs(stats.dailyProfitLoss).toLocaleString()}
            </div>
            <div className={`ml-2 flex items-center ${stats.dailyProfitLoss >= 0 ? 'text-green-600' : 'text-red-600'}`}>
              {stats.dailyProfitLoss >= 0 ? (
                <ArrowUpIcon className="h-4 w-4" />
              ) : (
                <ArrowDownIcon className="h-4 w-4" />
              )}
              {Math.abs(stats.dailyProfitLossPercentage).toFixed(2)}%
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}