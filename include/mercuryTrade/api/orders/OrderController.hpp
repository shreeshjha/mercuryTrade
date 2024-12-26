// include/mercuryTrade/api/orders/OrderController.hpp
#pragma once
#include "../../http/Server.hpp"
#include "../../services/OrderService.hpp"
#include <memory>

namespace mercuryTrade {
namespace api {
namespace orders {

class OrderController {
public:
    explicit OrderController(std::shared_ptr<OrderService> orderService);
    
    http::Response placeOrder(const http::Request& req);
    http::Response cancelOrder(const std::string& orderId);
    http::Response getOrders();
    http::Response getOrderById(const std::string& orderId);

private:
    std::shared_ptr<OrderService> m_orderService;
};

}}}