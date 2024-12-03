type OrderBookEntry = {
    price: number;
    size: number;
    total: number;
  }
  
  export default function OrderBook() {
    const asks: OrderBookEntry[] = [
      { price: 50000, size: 1.2, total: 1.2 },
      { price: 49900, size: 0.8, total: 2.0 },
      { price: 49800, size: 1.5, total: 3.5 },
    ].reverse();
  
    const bids: OrderBookEntry[] = [
      { price: 49700, size: 1.0, total: 1.0 },
      { price: 49600, size: 2.0, total: 3.0 },
      { price: 49500, size: 1.5, total: 4.5 },
    ];
  
    return (
      <div className="w-full">
        <div className="grid grid-cols-3 gap-2 text-xs text-gray-500 mb-1">
          <div>Price</div>
          <div>Size</div>
          <div>Total</div>
        </div>
        
        {/* Asks */}
        <div className="space-y-1">
          {asks.map((ask, i) => (
            <div key={i} className="grid grid-cols-3 gap-2 text-sm">
              <div className="text-red-500">${ask.price.toLocaleString()}</div>
              <div>{ask.size.toFixed(3)}</div>
              <div>{ask.total.toFixed(3)}</div>
            </div>
          ))}
        </div>
  
        {/* Spread */}
        <div className="my-2 text-sm text-gray-500 border-y border-gray-200 py-1">
          Spread: $300 (0.60%)
        </div>
  
        {/* Bids */}
        <div className="space-y-1">
          {bids.map((bid, i) => (
            <div key={i} className="grid grid-cols-3 gap-2 text-sm">
              <div className="text-green-500">${bid.price.toLocaleString()}</div>
              <div>{bid.size.toFixed(3)}</div>
              <div>{bid.total.toFixed(3)}</div>
            </div>
          ))}
        </div>
      </div>
    );
  }