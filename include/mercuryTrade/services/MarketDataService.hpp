// include/mercuryTrade/services/MarketDataService.hpp
#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace mercuryTrade {

struct MarketData {
    std::string symbol;
    double bid;
    double ask;
    double last;
    double volume;
    long timestamp;

    nlohmann::json toJson() const {
        return {
            {"symbol", symbol},
            {"bid", bid},
            {"ask", ask},
            {"last", last},
            {"volume", volume},
            {"timestamp", timestamp}
        };
    }
};

class MarketDataService {
public:
    MarketData getMarketData(const std::string& symbol);
    std::vector<std::string> getAvailableSymbols() const;
private:
    // In a real implementation, this would maintain connections to market data feeds
};

} // namespace mercuryTrade