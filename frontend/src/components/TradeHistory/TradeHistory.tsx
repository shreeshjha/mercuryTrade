import { useTradingContext } from '../../contexts/TradingContext';

export default function TradeHistory() {
  const { trades } = useTradingContext();

  return (
    <div>
      <div className="grid grid-cols-4 gap-2 text-xs text-gray-500 mb-1">
        <div>Price</div>
        <div>Size</div>
        <div>Side</div>
        <div>Time</div>
      </div>
      
      <div className="space-y-1">
        {trades.map((trade) => (
          <div key={trade.id} className="grid grid-cols-4 gap-2 text-sm">
            <div className={trade.side === 'buy' ? 'text-green-500' : 'text-red-500'}>
              ${trade.price.toLocaleString()}
            </div>
            <div>{trade.quantity.toFixed(3)}</div>
            <div className={trade.side === 'buy' ? 'text-green-500' : 'text-red-500'}>
              {trade.side.toUpperCase()}
            </div>
            <div>
              {new Date(trade.timestamp).toLocaleTimeString()}
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}