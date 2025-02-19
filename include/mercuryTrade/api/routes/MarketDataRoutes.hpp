#pragma once

#include <crow.h>
#include "../market/MarketDataController.hpp"

namespace mercuryTrade {
namespace api {
namespace routes {

void registerMarketDataRoutes(crow::SimpleApp& app, std::shared_ptr<market::MarketDataController> controller) {
    CROW_ROUTE(app, "/api/market-data/<string>").methods(crow::HTTPMethod::GET)
    ([controller](const std::string& symbol) {
        return controller->getMarketData(symbol);
    });

    CROW_ROUTE(app, "/api/order-book/<string>").methods(crow::HTTPMethod::GET)
    ([controller](const std::string& symbol) {
        return controller->getOrderBook(symbol);
    });
}

}}} // namespace