#pragma once

#include <crow.h>
#include "../orders/OrderController.hpp"

namespace mercuryTrade {
namespace api {
namespace routes {

void registerOrderRoutes(crow::SimpleApp& app, std::shared_ptr<orders::OrderController> controller) {
    CROW_ROUTE(app, "/api/orders")
        .methods(crow::HTTPMethod::GET)
    ([controller](const crow::request& req) {
        return controller->getOrders();
    });

    CROW_ROUTE(app, "/api/orders")
        .methods(crow::HTTPMethod::POST)
    ([controller](const crow::request& req) {
        return controller->placeOrder(req);
    });

    CROW_ROUTE(app, "/api/orders/<string>")
        .methods(crow::HTTPMethod::GET)
    ([controller](const std::string& orderId) {
        return controller->getOrderById(orderId);
    });

    CROW_ROUTE(app, "/api/orders/<string>")
        .methods(crow::HTTPMethod::DELETE)
    ([controller](const std::string& orderId) {
        return controller->cancelOrder(orderId);
    });
}

}}} // namespace