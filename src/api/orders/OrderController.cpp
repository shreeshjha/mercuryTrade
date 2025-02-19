// src/api/orders/OrderController.cpp
#include "../../../include/mercuryTrade/api/orders/OrderController.hpp"

namespace mercuryTrade {
namespace api {
namespace orders {

OrderController::OrderController(std::shared_ptr<OrderService> orderService)
    : m_orderService(orderService) {}

http::Response OrderController::placeOrder(const http::Request& req) {
    try {
        auto data = nlohmann::json::parse(req.body);
        
        Order order;
        order.symbol = data["symbol"].get<std::string>();
        order.side = data["side"].get<std::string>() == "buy" ? OrderSide::Buy : OrderSide::Sell;
        order.type = data["type"].get<std::string>() == "market" ? OrderType::Market : OrderType::Limit;
        order.quantity = data["quantity"].get<double>();
        
        if (order.type == OrderType::Limit) {
            order.price = data["price"].get<double>();
        }

        auto placedOrder = m_orderService->placeOrder(order);
        return http::Response::json(placedOrder.toJson(), 201);
    } catch (const std::exception& e) {
        return http::Response::json({{"error", e.what()}}, 400);
    }
}

http::Response OrderController::cancelOrder(const std::string& orderId) {
    try {
        m_orderService->cancelOrder(orderId);
        return http::Response::json({{"status", "success"}});
    } catch (const std::exception& e) {
        return http::Response::json({{"error", e.what()}}, 400);
    }
}

http::Response OrderController::getOrders() {
    try {
        auto orders = m_orderService->getOrders();
        nlohmann::json response = nlohmann::json::array();
        for (const auto& order : orders) {
            response.push_back(order.toJson());
        }
        return http::Response::json(response);
    } catch (const std::exception& e) {
        return http::Response::json({{"error", e.what()}}, 400);
    }
}

http::Response OrderController::getOrderById(const std::string& orderId) {
    try {
        auto order = m_orderService->getOrderById(orderId);
        if (!order) {
            return http::Response::json({{"error", "Order not found"}}, 404);
        }
        return http::Response::json(order->toJson());
    } catch (const std::exception& e) {
        return http::Response::json({{"error", e.what()}}, 400);
    }
}

}}} // namespace