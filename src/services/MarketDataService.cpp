// src/services/MarketDataService.cpp
#include "../../include/mercuryTrade/services/MarketDataService.hpp"
#include <chrono>

namespace mercuryTrade {

MarketData MarketDataService::getMarketData(const std::string& symbol) {
    // In a real implementation, this would fetch real market data
    // This is just a mock implementation
    MarketData data;
    data.symbol = symbol;
    data.bid = 50000.0;
    data.ask = 50100.0;
    data.last = 50050.0;
    data.volume = 100.0;
    data.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    
    return data;
}

std::vector<std::string> MarketDataService::getAvailableSymbols() const {
    return {"BTC-USD", "ETH-USD", "SOL-USD"};
}

} // namespace