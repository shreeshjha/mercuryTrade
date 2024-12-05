import { usePortfolio } from "../../contexts/PortfolioContext";

export function TransactionList() {
    const { transactions, loading, error } = usePortfolio();
  
    if (loading) return <div className="text-gray-500">Loading transactions...</div>;
    if (error) return <div className="text-red-500">{error}</div>;
    if (!loading && transactions.length === 0) {
        return <div className="text-gray-500">No transactions found.</div>;
      }
      if (error) {
        console.error('Portfolio error:', error); // Debug log
        return <div className="text-red-500">{error}</div>;
      }

    return (
      <div className="space-y-2">
        {transactions.map((tx) => (
          <div key={tx.id} className="bg-white rounded-lg p-4 shadow">
            <div className="flex justify-between items-center">
              <div>
                <div className="font-medium capitalize">{tx.type}</div>
                <div className="text-sm text-gray-500">
                  {tx.symbol} • {new Date(tx.timestamp).toLocaleString()}
                </div>
              </div>
              <div className="text-right">
                <div className="font-medium">
                  {tx.amount.toFixed(8)} {tx.symbol}
                </div>
                <div className="text-sm text-gray-500">
                  Fee: {tx.fee?.toFixed(8) ?? '0'} {tx.symbol}
                </div>
              </div>
            </div>
          </div>
        ))}
      </div>
    );
  }