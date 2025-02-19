// include/mercuryTrade/services/OrderBookService.hpp
#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace mercuryTrade {

struct OrderBookLevel {
    double price;
    double quantity;
    
    nlohmann::json toJson() const {
        return {
            {"price", price},
            {"quantity", quantity}
        };
    }
};

struct OrderBook {
    std::string symbol;
    std::vector<OrderBookLevel> bids;
    std::vector<OrderBookLevel> asks;
    long timestamp;

    nlohmann::json toJson() const {
        nlohmann::json j;
        j["symbol"] = symbol;
        j["timestamp"] = timestamp;

        auto& bidsJson = j["bids"] = nlohmann::json::array();
        auto& asksJson = j["asks"] = nlohmann::json::array();

        for (const auto& bid : bids) bidsJson.push_back(bid.toJson());
        for (const auto& ask : asks) asksJson.push_back(ask.toJson());

        return j;
    }
};

class OrderBookService {
public:
    OrderBook getOrderBook(const std::string& symbol);
    void updateOrderBook(const std::string& symbol, const OrderBook& book);
private:
    // Would maintain real-time order book state
};

} // namespace mercuryTrade