import axios from 'axios';
import { Order, MarketData, Trade } from '../types/trading';

const API_BASE_URL = 'http://localhost:3000/api';

export const tradingApi = {
  // Order related endpoints
  submitOrder: async (order: Omit<Order, 'id' | 'timestamp' | 'status'>) => {
    const response = await axios.post(`${API_BASE_URL}/orders`, order);
    return response.data;
  },

  cancelOrder: async (orderId: string) => {
    const response = await axios.delete(`${API_BASE_URL}/orders/${orderId}`);
    return response.data;
  },

  // Market data endpoints
  getMarketData: async (symbol: string) => {
    const response = await axios.get<MarketData>(`${API_BASE_URL}/market-data/${symbol}`);
    return response.data;
  },

  // Trade history endpoints
  getTradeHistory: async (symbol: string) => {
    const response = await axios.get<Trade[]>(`${API_BASE_URL}/trades/${symbol}`);
    return response.data;
  },

  getPriceHistory: async (symbol: string) => {
    const response = await axios.get(`${API_BASE_URL}/market-data/${symbol}/history`);
    return response.data;
  },

  getOrderBook: async (symbol: string) => {
    const response = await axios.get(`${API_BASE_URL}/market-data/${symbol}/order-book`);
    return response.data;
  },
};