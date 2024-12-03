import { useState } from 'react';

export default function OrderForm() {
  const [orderType, setOrderType] = useState<'market' | 'limit'>('limit');
  const [side, setSide] = useState<'buy' | 'sell'>('buy');
  const [price, setPrice] = useState('');
  const [quantity, setQuantity] = useState('');

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    // Handle order submission
    console.log({ orderType, side, price, quantity });
  };

  return (
    <form onSubmit={handleSubmit} className="space-y-4">
      <div className="flex space-x-4">
        <button
          type="button"
          className={`flex-1 py-2 px-4 rounded-lg font-medium ${
            side === 'buy' 
              ? 'bg-green-500 text-white' 
              : 'bg-gray-200 text-gray-700'
          }`}
          onClick={() => setSide('buy')}
        >
          Buy
        </button>
        <button
          type="button"
          className={`flex-1 py-2 px-4 rounded-lg font-medium ${
            side === 'sell' 
              ? 'bg-red-500 text-white' 
              : 'bg-gray-200 text-gray-700'
          }`}
          onClick={() => setSide('sell')}
        >
          Sell
        </button>
      </div>

      <div className="flex space-x-4">
        <button
          type="button"
          className={`flex-1 py-2 px-4 rounded-lg font-medium ${
            orderType === 'limit' 
              ? 'bg-blue-500 text-white' 
              : 'bg-gray-200 text-gray-700'
          }`}
          onClick={() => setOrderType('limit')}
        >
          Limit
        </button>
        <button
          type="button"
          className={`flex-1 py-2 px-4 rounded-lg font-medium ${
            orderType === 'market' 
              ? 'bg-blue-500 text-white' 
              : 'bg-gray-200 text-gray-700'
          }`}
          onClick={() => setOrderType('market')}
        >
          Market
        </button>
      </div>

      {orderType === 'limit' && (
        <div>
          <label className="block text-sm font-medium text-gray-700">Price</label>
          <input
            type="number"
            value={price}
            onChange={(e) => setPrice(e.target.value)}
            className="mt-1 block w-full rounded-md border-gray-300 shadow-sm focus:border-blue-500 focus:ring-blue-500"
            placeholder="Enter price"
          />
        </div>
      )}

      <div>
        <label className="block text-sm font-medium text-gray-700">Quantity</label>
        <input
          type="number"
          value={quantity}
          onChange={(e) => setQuantity(e.target.value)}
          className="mt-1 block w-full rounded-md border-gray-300 shadow-sm focus:border-blue-500 focus:ring-blue-500"
          placeholder="Enter quantity"
        />
      </div>

      <button
        type="submit"
        className={`w-full py-2 px-4 rounded-lg font-medium text-white ${
          side === 'buy' ? 'bg-green-500' : 'bg-red-500'
        }`}
      >
        {side === 'buy' ? 'Buy' : 'Sell'} BTC
      </button>
    </form>
  );
}