// include/mercuryTrade/api/market/MarketDataController.hpp
#pragma once

#include <memory>
#include <crow.h>
#include "../../services/MarketDataService.hpp"
#include "../../services/OrderBookService.hpp"

namespace mercuryTrade {
namespace api {
namespace market {

class MarketDataController {
public:
    explicit MarketDataController(
        std::shared_ptr<MarketDataService> marketDataService,
        std::shared_ptr<OrderBookService> orderBookService);

    crow::response getMarketData(const std::string& symbol);
    crow::response getOrderBook(const std::string& symbol);

private:
    std::shared_ptr<MarketDataService> m_marketDataService;
    std::shared_ptr<OrderBookService> m_orderBookService;
<<<<<<< HEAD
};
=======
};
>>>>>>> 66198280ff132809c83603358161bc8bb029aa5e
