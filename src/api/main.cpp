#include "mercuryTrade/http/Server.hpp"
#include "mercuryTrade/api/auth/AuthController.hpp"
#include "mercuryTrade/api/market/MarketDataController.hpp"
#include "mercuryTrade/api/orders/OrderController.hpp"
#include <memory>

int main() {
    auto userService = std::make_shared<mercuryTrade::UserService>();
    auto marketDataService = std::make_shared<mercuryTrade::MarketDataService>();
    auto orderBookService = std::make_shared<mercuryTrade::OrderBookService>();
    auto orderService = std::make_shared<mercuryTrade::OrderService>();


    auto authController = std::make_shared<mercuryTrade::api::auth::AuthController>(userService);
    auto marketDataController = std::make_shared<mercuryTrade::api::market::MarketDataController>(
        marketDataService, orderBookService);
    auto orderController = std::make_shared<mercuryTrade::api::orders::OrderController>(orderService);

    mercuryTrade::http::Server server(3000);


    server.post("/api/auth/login", [&](const mercuryTrade::http::Request& req) { 
        return authController->login(req); 
    });
    server.post("/api/auth/register", [&](const mercuryTrade::http::Request& req) { 
        return authController->registerUser(req); 
    });
    server.post("/api/auth/logout", [&](const mercuryTrade::http::Request& req) { 
        return authController->logout(req); 
    });


    server.get("/api/market-data/{symbol}", [&](const mercuryTrade::http::Request& req) { 
        return marketDataController->getMarketData(req.getParam("symbol")); 
    });
    
    server.get("/api/order-book/{symbol}", [&](const mercuryTrade::http::Request& req) { 
        return marketDataController->getOrderBook(req.getParam("symbol")); 
    });


    server.get("/api/orders", [&](const mercuryTrade::http::Request& req) { 
        return orderController->getOrders(); 
    });
    
    server.get("/api/orders/{id}", [&](const mercuryTrade::http::Request& req) { 
        return orderController->getOrderById(req.getParam("id")); 
    });
    
    server.post("/api/orders", [&](const mercuryTrade::http::Request& req) { 
        return orderController->placeOrder(req); 
    });

    server.start();
    return 0;
}