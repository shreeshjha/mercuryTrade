// src/services/OrderBookService.cpp
#include "../../include/mercuryTrade/services/OrderBookService.hpp"
#include <chrono>

namespace mercuryTrade {

OrderBook OrderBookService::getOrderBook(const std::string& symbol) {
    // Mock implementation
    OrderBook book;
    book.symbol = symbol;
    book.timestamp = std::chrono::system_clock::now().time_since_epoch().count();

    // Add some sample bids
    book.bids = {
        {49990.0, 1.5},
        {49980.0, 2.0},
        {49970.0, 2.5}
    };

    // Add some sample asks
    book.asks = {
        {50010.0, 1.0},
        {50020.0, 1.8},
        {50030.0, 2.2}
    };

    return book;
}

void OrderBookService::updateOrderBook(const std::string& symbol, const OrderBook& book) {
    // Would update internal order book state
}

} // namespace