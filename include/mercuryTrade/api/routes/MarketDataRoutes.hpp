#pragma once

#include <crow.h>
#include "../market/MarketDataController.hpp"

namespace mercuryTrade {
namespace api {
namespace routes {

void registerMarketDataRoutes(crow::App<crow::CORSHandler>& app, 
    std::shared_ptr<market::MarketDataController> marketDataController);

}}}
