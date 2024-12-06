#pragma once
#include <string>
#include <vector>

struct OrderBookEntry {
    double price;
    double quantity;
};

struct OrderBook {
    std::vector<OrderBookEntry> bids;
    std::vector<OrderBookEntry> asks;
    long timestamp;
};

class OrderBookService {
public:
    OrderBook getOrderBook(const std::string& symbol);
};