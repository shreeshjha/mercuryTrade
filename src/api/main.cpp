// src/api/main.cpp
<<<<<<< HEAD
#include "crow.h"
#include "mercuryTrade/api/routes/AuthRoutes.hpp"
#include "mercuryTrade/api/routes/MarketDataRoutes.hpp"
=======
#include <crow.h>
#include "../../include/mercuryTrade/api/routes/AuthRoutes.hpp"
#include "../../include/mercuryTrade/api/routes/MarketDataRoutes.hpp"
>>>>>>> 66198280ff132809c83603358161bc8bb029aa5e

int main() {
    crow::App<crow::CORSHandler> app;

    // Configure CORS
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors
        .global()
        .headers("*")
        .methods("GET"_method, "POST"_method, "PUT"_method, "DELETE"_method);

    // Initialize services and controllers
    auto userService = std::make_shared<UserService>();
    auto marketDataService = std::make_shared<MarketDataService>();
    auto orderBookService = std::make_shared<OrderBookService>();

    auto authController = std::make_shared<auth::AuthController>(userService);
    auto marketDataController = std::make_shared<market::MarketDataController>(
        marketDataService, orderBookService);

    // Register routes
    mercuryTrade::api::routes::registerAuthRoutes(app, authController);
    mercuryTrade::api::routes::registerMarketDataRoutes(app, marketDataController);

    app.port(3000).multithreaded().run();
    return 0;
<<<<<<< HEAD
}
=======
}
>>>>>>> 66198280ff132809c83603358161bc8bb029aa5e
