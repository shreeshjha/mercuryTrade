// src/pages/Positions.tsx
import React from 'react';
import PositionsTable from '../components/Positions/PositionsTable';
import { PositionsSummary } from '../components/Positions/PositionsSummary';

export default function Positions() {
  return (
    <div className="p-4 space-y-6">
      <h1 className="text-2xl font-bold text-gray-900">Positions</h1>
      <PositionsSummary />
      <div className="bg-white rounded-lg shadow">
        <div className="p-4">
          <h2 className="text-lg font-medium text-gray-900 mb-4">Active Positions</h2>
          <PositionsTable />
        </div>
      </div>
    </div>
  );
}