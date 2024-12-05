import { usePortfolio } from "../../contexts/PortfolioContext";

export function AssetList() {
    const { balances, loading, error } = usePortfolio();
  
    if (loading) return <div className="text-gray-500">Loading assets...</div>;
    if (error) return <div className="text-red-500">{error}</div>;
  
    return (
      <div className="space-y-4">
        {balances.map((asset) => (
          <div key={asset.symbol} className="bg-white rounded-lg p-4 shadow">
            <div className="flex justify-between items-center">
              <div>
                <div className="font-medium">{asset.symbol}</div>
                <div className="text-sm text-gray-500">
                  Available: {asset.available.toFixed(8)}
                </div>
              </div>
              <div className="text-right">
                <div className="font-medium">${asset.valueInUSD.toLocaleString()}</div>
                <div className="text-sm text-gray-500">
                  Locked: {asset.locked.toFixed(8)}
                </div>
              </div>
            </div>
          </div>
        ))}
      </div>
    );
  }