// include/mercuryTrade/services/OrderService.hpp
#pragma once
#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace mercuryTrade {

enum class OrderSide {
    Buy,
    Sell
};

enum class OrderType {
    Market,
    Limit
};

enum class OrderStatus {
    New,
    PartiallyFilled,
    Filled,
    Cancelled,
    Rejected
};

struct Order {
    std::string id;
    std::string symbol;
    OrderSide side;
    OrderType type;
    double quantity;
    double price;  // Only used for limit orders
    OrderStatus status;
    long timestamp;

    nlohmann::json toJson() const {
        return {
            {"id", id},
            {"symbol", symbol},
            {"side", side == OrderSide::Buy ? "buy" : "sell"},
            {"type", type == OrderType::Market ? "market" : "limit"},
            {"quantity", quantity},
            {"price", price},
            {"status", static_cast<int>(status)},
            {"timestamp", timestamp}
        };
    }
};

class OrderService {
public:
    Order placeOrder(const Order& order);
    void cancelOrder(const std::string& orderId);
    std::vector<Order> getOrders(const std::string& symbol = "");
    std::optional<Order> getOrderById(const std::string& orderId);

private:
    // Would maintain order state and connect to exchange
    std::vector<Order> orders_;
};

} // namespace mercuryTrade