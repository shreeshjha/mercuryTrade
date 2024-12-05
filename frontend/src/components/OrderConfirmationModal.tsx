import React from 'react';

interface OrderDetails {
  type: 'market' | 'limit';
  side: 'buy' | 'sell';
  price?: number;
  quantity: number;
  total: number;
  symbol: string;
}

interface OrderConfirmationModalProps {
  isOpen: boolean;
  onClose: () => void;
  onConfirm: () => void;
  orderDetails: OrderDetails;
}

export function OrderConfirmationModal({
  isOpen,
  onClose,
  onConfirm,
  orderDetails
}: OrderConfirmationModalProps) {
  if (!isOpen) return null;

  const { type, side, price, quantity, total, symbol } = orderDetails;

  return (
    <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
      <div className="bg-white rounded-lg p-6 max-w-md w-full mx-4">
        {/* Header */}
        <div className="mb-6">
          <h3 className="text-lg font-medium text-gray-900">
            Confirm {side === 'buy' ? 'Buy' : 'Sell'} Order
          </h3>
        </div>

        {/* Content */}
        <div className="space-y-4">
          <div className="grid grid-cols-2 gap-4 text-sm">
            <div className="text-gray-500">Order Type</div>
            <div className="font-medium capitalize">{type}</div>

            <div className="text-gray-500">Symbol</div>
            <div className="font-medium">{symbol}</div>

            <div className="text-gray-500">Side</div>
            <div className={`font-medium capitalize ${
              side === 'buy' ? 'text-green-600' : 'text-red-600'
            }`}>
              {side}
            </div>

            {type === 'limit' && (
              <>
                <div className="text-gray-500">Price</div>
                <div className="font-medium">${price?.toLocaleString()}</div>
              </>
            )}

            <div className="text-gray-500">Quantity</div>
            <div className="font-medium">{quantity.toFixed(8)} {symbol.split('-')[0]}</div>

            <div className="text-gray-500">Total</div>
            <div className="font-medium">${total.toLocaleString()}</div>
          </div>

          <div className="bg-gray-50 p-4 rounded-lg text-sm text-gray-600">
            Please review your order details carefully. This action cannot be undone.
          </div>
        </div>

        {/* Footer */}
        <div className="mt-6 flex justify-end space-x-3">
          <button
            onClick={onClose}
            className="px-4 py-2 text-sm font-medium text-gray-700 bg-gray-100 hover:bg-gray-200 rounded-lg"
          >
            Cancel
          </button>
          <button
            onClick={onConfirm}
            className={`px-4 py-2 text-sm font-medium text-white rounded-lg ${
              side === 'buy' 
                ? 'bg-green-600 hover:bg-green-700' 
                : 'bg-red-600 hover:bg-red-700'
            }`}
          >
            Confirm {side === 'buy' ? 'Buy' : 'Sell'}
          </button>
        </div>
      </div>
    </div>
  );
}