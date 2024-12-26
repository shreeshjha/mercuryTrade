// src/services/OrderService.cpp
#include "../../include/mercuryTrade/services/OrderService.hpp"
#include <chrono>
#include <random>

namespace mercuryTrade {

Order OrderService::placeOrder(const Order& order) {
    // Generate a unique order ID
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);
    
    Order newOrder = order;
    newOrder.id = "ORD-" + std::to_string(dis(gen));
    newOrder.status = OrderStatus::New;
    newOrder.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    
    orders_.push_back(newOrder);
    return newOrder;
}

void OrderService::cancelOrder(const std::string& orderId) {
    for (auto& order : orders_) {
        if (order.id == orderId) {
            order.status = OrderStatus::Cancelled;
            return;
        }
    }
    throw std::runtime_error("Order not found");
}

std::vector<Order> OrderService::getOrders(const std::string& symbol) {
    if (symbol.empty()) {
        return orders_;
    }
    
    std::vector<Order> filtered;
    std::copy_if(orders_.begin(), orders_.end(), std::back_inserter(filtered),
        [&](const Order& order) { return order.symbol == symbol; });
    return filtered;
}

std::optional<Order> OrderService::getOrderById(const std::string& orderId) {
    auto it = std::find_if(orders_.begin(), orders_.end(),
        [&](const Order& order) { return order.id == orderId; });
    
    if (it != orders_.end()) {
        return *it;
    }
    return std::nullopt;
}

} // namespace