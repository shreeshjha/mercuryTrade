// include/mercuryTrade/api/market/MarketDataController.hpp
#pragma once
#include "../../http/Server.hpp"
#include "../../services/MarketDataService.hpp"
#include "../../services/OrderBookService.hpp"
#include <memory>

namespace mercuryTrade {
namespace api {
namespace market {

class MarketDataController {
public:
    explicit MarketDataController(
        std::shared_ptr<MarketDataService> marketDataService,
        std::shared_ptr<OrderBookService> orderBookService);
    
    http::Response getMarketData(const std::string& symbol);
    http::Response getOrderBook(const std::string& symbol);

private:
    std::shared_ptr<MarketDataService> m_marketDataService;
    std::shared_ptr<OrderBookService> m_orderBookService;
};

}}}