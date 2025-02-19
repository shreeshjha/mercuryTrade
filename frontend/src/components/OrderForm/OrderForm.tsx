import React, { useState, useEffect } from 'react';
import { useOrderManagement, useMarketData } from '../../contexts/TradingContext';

const OrderForm = () => {
  const { marketData } = useMarketData();
  const { submitOrder, loading, error } = useOrderManagement();

  const [orderType, setOrderType] = useState<'market' | 'limit'>('limit');
  const [side, setSide] = useState<'buy' | 'sell'>('buy');
  const [price, setPrice] = useState('');
  const [quantity, setQuantity] = useState('');
  const [total, setTotal] = useState<number>(0);

  // Update price when market data changes
  useEffect(() => {
    if (orderType === 'market' && marketData) {
      setPrice(side === 'buy' ? marketData.ask.toString() : marketData.bid.toString());
    }
  }, [orderType, side, marketData]);

  // Calculate total when price or quantity changes
  useEffect(() => {
    const priceNum = parseFloat(price) || 0;
    const quantityNum = parseFloat(quantity) || 0;
    setTotal(priceNum * quantityNum);
  }, [price, quantity]);

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      await submitOrder({
        symbol: marketData?.symbol || 'BTC-USD',
        type: orderType,
        side,
        price: orderType === 'limit' ? parseFloat(price) : undefined,
        quantity: parseFloat(quantity)
      });

      // Reset form on success
      setQuantity('');
      if (orderType === 'limit') {
        setPrice('');
      }
    } catch (err) {
      console.error('Failed to submit order:', err);
    }
  };

  return (
    <form onSubmit={handleSubmit} className="space-y-6">
      {/* Order Type Selection */}
      <div className="flex space-x-2">
        <button
          type="button"
          onClick={() => setOrderType('limit')}
          className={`flex-1 py-2 px-4 rounded-lg font-medium ${
            orderType === 'limit'
              ? 'bg-blue-600 text-white'
              : 'bg-gray-100 text-gray-600 hover:bg-gray-200'
          }`}
        >
          Limit
        </button>
        <button
          type="button"
          onClick={() => setOrderType('market')}
          className={`flex-1 py-2 px-4 rounded-lg font-medium ${
            orderType === 'market'
              ? 'bg-blue-600 text-white'
              : 'bg-gray-100 text-gray-600 hover:bg-gray-200'
          }`}
        >
          Market
        </button>
      </div>

      {/* Buy/Sell Selection */}
      <div className="flex space-x-2">
        <button
          type="button"
          onClick={() => setSide('buy')}
          className={`flex-1 py-2 px-4 rounded-lg font-medium ${
            side === 'buy'
              ? 'bg-green-600 text-white'
              : 'bg-gray-100 text-gray-600 hover:bg-gray-200'
          }`}
        >
          Buy
        </button>
        <button
          type="button"
          onClick={() => setSide('sell')}
          className={`flex-1 py-2 px-4 rounded-lg font-medium ${
            side === 'sell'
              ? 'bg-red-600 text-white'
              : 'bg-gray-100 text-gray-600 hover:bg-gray-200'
          }`}
        >
          Sell
        </button>
      </div>

      {/* Price Input */}
      {orderType === 'limit' && (
        <div>
          <label className="block text-sm font-medium text-gray-700 mb-1">
            Price (USD)
          </label>
          <input
            type="number"
            value={price}
            onChange={(e) => setPrice(e.target.value)}
            step="0.01"
            min="0"
            required
            className="block w-full px-3 py-2 rounded-lg border border-gray-300 shadow-sm focus:ring-blue-500 focus:border-blue-500"
            placeholder="Enter price"
          />
        </div>
      )}

      {/* Quantity Input */}
      <div>
        <label className="block text-sm font-medium text-gray-700 mb-1">
          Amount (BTC)
        </label>
        <input
          type="number"
          value={quantity}
          onChange={(e) => setQuantity(e.target.value)}
          step="0.00000001"
          min="0"
          required
          className="block w-full px-3 py-2 rounded-lg border border-gray-300 shadow-sm focus:ring-blue-500 focus:border-blue-500"
          placeholder="Enter amount"
        />
      </div>

      {/* Order Summary */}
      <div className="bg-gray-50 p-4 rounded-lg">
        <div className="flex justify-between text-sm">
          <span className="text-gray-500">Total</span>
          <span className="font-medium">${total.toLocaleString()}</span>
        </div>
        {marketData && (
          <div className="flex justify-between text-sm mt-2">
            <span className="text-gray-500">Best {side === 'buy' ? 'ask' : 'bid'}</span>
            <span className="font-medium">
              ${(side === 'buy' ? marketData.ask : marketData.bid).toLocaleString()}
            </span>
          </div>
        )}
      </div>

      {error && (
        <div className="bg-red-50 text-red-600 p-3 rounded-lg text-sm">
          {error}
        </div>
      )}

      {/* Submit Button */}
      <button
        type="submit"
        disabled={loading}
        className={`w-full py-3 px-4 rounded-lg font-medium text-white ${
          side === 'buy'
            ? 'bg-green-600 hover:bg-green-700'
            : 'bg-red-600 hover:bg-red-700'
        } disabled:opacity-50 disabled:cursor-not-allowed`}
      >
        {loading ? 'Processing...' : `${side === 'buy' ? 'Buy' : 'Sell'} Bitcoin`}
      </button>
    </form>
  );
};

export default OrderForm;