
import React from 'react';
import { PortfolioOverview } from '../components/Portfolio/PortfolioOverview';
import { AssetList } from '../components/Portfolio/AssetList';
import { TransactionList } from '../components/Portfolio/TransactionList';


export default function Portfolio() {
  return (
    <div className="space-y-6 p-4">
      <h1 className="text-2xl font-bold text-gray-900">Portfolio</h1>
      
      <PortfolioOverview />

      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        <div>
          <h2 className="text-lg font-medium text-gray-900 mb-4">Assets</h2>
          <AssetList />
        </div>
        
        <div>
          <h2 className="text-lg font-medium text-gray-900 mb-4">Recent Transactions</h2>
          <TransactionList />
        </div>
      </div>
    </div>
  );
}