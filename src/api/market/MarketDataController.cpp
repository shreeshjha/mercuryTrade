// src/api/market/MarketDataController.cpp
#include "../../../include/mercuryTrade/api/market/MarketDataController.hpp"

namespace mercuryTrade {
namespace api {
namespace market {

MarketDataController::MarketDataController(
    std::shared_ptr<MarketDataService> marketDataService,
    std::shared_ptr<OrderBookService> orderBookService)
    : m_marketDataService(marketDataService)
    , m_orderBookService(orderBookService) {}

http::Response MarketDataController::getMarketData(const std::string& symbol) {
    try {
        auto marketData = m_marketDataService->getMarketData(symbol);
        return http::Response::json(marketData.toJson());
    } catch (const std::exception& e) {
        return http::Response::json({{"error", e.what()}}, 400);
    }
}

http::Response MarketDataController::getOrderBook(const std::string& symbol) {
    try {
        auto orderBook = m_orderBookService->getOrderBook(symbol);
        return http::Response::json(orderBook.toJson());
    } catch (const std::exception& e) {
        return http::Response::json({{"error", e.what()}}, 400);
    }
}

}}} // namespace