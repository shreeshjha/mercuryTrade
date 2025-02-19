import React from 'react';
import { usePositions } from '../../contexts/PositionsContext';
import { XMarkIcon } from '@heroicons/react/24/outline';

export default function PositionsTable() {
  const { positions, closePosition, loading, error } = usePositions();

  if (loading) return <div className="text-gray-500">Loading positions...</div>;
  if (error) return <div className="text-red-500">{error}</div>;

  return (
    <div className="overflow-x-auto">
      <table className="min-w-full divide-y divide-gray-200">
        <thead className="bg-gray-50">
          <tr>
            <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">Symbol</th>
            <th className="px-6 py-3 text-right text-xs font-medium text-gray-500 uppercase">Size</th>
            <th className="px-6 py-3 text-right text-xs font-medium text-gray-500 uppercase">Entry Price</th>
            <th className="px-6 py-3 text-right text-xs font-medium text-gray-500 uppercase">Mark Price</th>
            <th className="px-6 py-3 text-right text-xs font-medium text-gray-500 uppercase">PnL</th>
            <th className="px-6 py-3 text-right text-xs font-medium text-gray-500 uppercase">Actions</th>
          </tr>
        </thead>
        <tbody className="divide-y divide-gray-200">
          {positions.map((position) => (
            <tr key={position.symbol}>
              <td className="px-6 py-4 whitespace-nowrap">
                <div className="flex items-center">
                  <span className={`inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium
                    ${position.side === 'long' ? 'bg-green-100 text-green-800' : 'bg-red-100 text-red-800'}`}>
                    {position.side}
                  </span>
                  <span className="ml-2">{position.symbol}</span>
                </div>
              </td>
              <td className="px-6 py-4 whitespace-nowrap text-right">
                {position.size.toFixed(4)}
              </td>
              <td className="px-6 py-4 whitespace-nowrap text-right">
                ${position.entryPrice.toLocaleString()}
              </td>
              <td className="px-6 py-4 whitespace-nowrap text-right">
                ${position.markPrice.toLocaleString()}
              </td>
              <td className={`px-6 py-4 whitespace-nowrap text-right font-medium
                ${position.unrealizedPnL >= 0 ? 'text-green-600' : 'text-red-600'}`}>
                ${position.unrealizedPnL.toLocaleString()}
                <span className="text-xs ml-1">
                  ({position.pnlPercentage.toFixed(2)}%)
                </span>
              </td>
              <td className="px-6 py-4 whitespace-nowrap text-right">
                <button
                  onClick={() => closePosition(position.symbol)}
                  className="text-gray-400 hover:text-red-600 transition-colors"
                >
                  <XMarkIcon className="h-5 w-5" />
                </button>
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}