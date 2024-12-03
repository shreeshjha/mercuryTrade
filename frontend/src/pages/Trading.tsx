import OrderBook from '../components/OrderBook/OrderBook';
import OrderForm from '../components/OrderForm/OrderForm';
import TradeHistory from '../components/TradeHistory/TradeHistory';
import PriceChart from '../components/PriceChart/PriceChart';

export default function Trading() {
  return (
    <div className="grid grid-cols-12 gap-4">
      {/* Left Column - Order Book */}
      <div className="col-span-3">
        <div className="bg-white rounded-lg shadow p-4">
          <h2 className="text-lg font-medium text-gray-900 mb-4">Order Book</h2>
          <OrderBook />
        </div>
      </div>

      {/* Middle Column - Chart and Order Form */}
      <div className="col-span-6">
        <div className="bg-white rounded-lg shadow p-4 mb-4">
          <h2 className="text-lg font-medium text-gray-900 mb-4">Price Chart</h2>
          <PriceChart />
        </div>
        <div className="bg-white rounded-lg shadow p-4">
          <h2 className="text-lg font-medium text-gray-900 mb-4">Place Order</h2>
          <OrderForm />
        </div>
      </div>

      {/* Right Column - Trade History */}
      <div className="col-span-3">
        <div className="bg-white rounded-lg shadow p-4">
          <h2 className="text-lg font-medium text-gray-900 mb-4">Trade History</h2>
          <TradeHistory />
        </div>
      </div>
    </div>
  );
}