#pragma once
#include <string>

struct MarketData {
    std::string symbol;
    double bid;
    double ask;
    double last;
    double volume;
    long timestamp;
};

class MarketDataService {
public:
    MarketData getMarketData(const std::string& symbol);
};