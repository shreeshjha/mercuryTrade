import { usePositions } from "../../contexts/PositionsContext";

export function PositionsSummary() {
    const { totalPnL, dailyPnL } = usePositions();
  
    return (
      <div className="grid grid-cols-2 gap-4">
        <div className="bg-white rounded-lg p-4 shadow">
          <div className="text-sm text-gray-500">Total PnL</div>
          <div className={`text-2xl font-bold ${totalPnL >= 0 ? 'text-green-600' : 'text-red-600'}`}>
            ${totalPnL.toLocaleString()}
          </div>
        </div>
        
        <div className="bg-white rounded-lg p-4 shadow">
          <div className="text-sm text-gray-500">Daily PnL</div>
          <div className={`text-2xl font-bold ${dailyPnL >= 0 ? 'text-green-600' : 'text-red-600'}`}>
            ${dailyPnL.toLocaleString()}
          </div>
        </div>
      </div>
    );
  }