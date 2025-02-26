// src/api/main.cpp
#include "mercuryTrade/http/Server.hpp"
#include "mercuryTrade/api/auth/AuthController.hpp"
#include "mercuryTrade/api/market/MarketDataController.hpp"
#include "mercuryTrade/api/orders/OrderController.hpp"
#include "mercuryTrade/services/UserService.hpp"
#include "mercuryTrade/services/MarketDataService.hpp"
#include "mercuryTrade/services/OrderBookService.hpp"
#include "mercuryTrade/services/OrderService.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>
#include <fstream>

using namespace mercuryTrade;
using json = nlohmann::json;

// Load configuration from file
json loadConfig(const std::string& configPath) {
    try {
        std::ifstream configFile(configPath);
        if (!configFile.is_open()) {
            throw std::runtime_error("Could not open config file: " + configPath);
        }
        
        json config;
        configFile >> config;
        return config;
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    std::string configPath = "config/config.json";
    if (argc > 1) {
        configPath = argv[1];
    }
    
    // Load configuration
    json config = loadConfig(configPath);
    
    // Initialize services
    auto userService = std::make_shared<UserService>();
    auto marketDataService = std::make_shared<MarketDataService>();
    auto orderBookService = std::make_shared<OrderBookService>();
    auto orderService = std::make_shared<OrderService>();
    
    // Initialize controllers
    auto authController = std::make_shared<api::auth::AuthController>(userService);
    auto marketDataController = std::make_shared<api::market::MarketDataController>(
        marketDataService, orderBookService);
    auto orderController = std::make_shared<api::orders::OrderController>(orderService);
    
    // Create HTTP server with JWT secret
    http::Server server(
        config["server"]["port"].get<int>(),
        config["jwt"]["secret"].get<std::string>()
    );
    
    // Configure protected routes
    server.use_auth_middleware("/api/orders");
    server.use_auth_middleware("/api/market-data");
    server.use_auth_middleware("/api/auth/profile");
    server.use_auth_middleware("/api/auth/update-password");
    
    // Auth routes
    server.post("/api/auth/login", [&](const http::Request& req) {
        return authController->login(req);
    });
    
    server.post("/api/auth/register", [&](const http::Request& req) {
        return authController->registerUser(req);
    });
    
    server.post("/api/auth/logout", [&](const http::Request& req) {
        return authController->logout(req);
    });
    
    server.get("/api/auth/validate", [&](const http::Request& req) {
        // This endpoint just returns user info if the JWT is valid
        if (req.user_id.empty()) {
            return http::Response::json({{"error", "Invalid or expired token"}}, 401);
        }
        
        // Return user info based on the token
        auto user = userService->getUserById(req.user_id);
        if (!user) {
            return http::Response::json({{"error", "User not found"}}, 404);
        }
        
        return http::Response::json(user->toJson());
    });
    
    // Market data routes
    server.get("/api/market-data/:symbol", [&](const http::Request& req) {
        return marketDataController->getMarketData(req.params["symbol"]);
    });
    
    server.get("/api/market-data/:symbol/order-book", [&](const http::Request& req) {
        return marketDataController->getOrderBook(req.params["symbol"]);
    });
    
    // Order routes
    server.post("/api/orders", [&](const http::Request& req) {
        return orderController->placeOrder(req);
    });
    
    server.get("/api/orders", [&](const http::Request& req) {
        return orderController->getOrders();
    });
    
    server.get("/api/orders/:id", [&](const http::Request& req) {
        return orderController->getOrderById(req.params["id"]);
    });
    
    server.del("/api/orders/:id", [&](const http::Request& req) {
        return orderController->cancelOrder(req.params["id"]);
    });
    
    // Start the server
    std::cout << "Starting Mercury Trade server on port " << config["server"]["port"] << std::endl;
    server.start();
    
    return 0;
}


// #include "mercuryTrade/http/Server.hpp"
// #include "mercuryTrade/api/auth/AuthController.hpp"
// #include "mercuryTrade/api/market/MarketDataController.hpp"
// #include "mercuryTrade/api/orders/OrderController.hpp"
// #include <memory>

// int main() {
//     auto userService = std::make_shared<mercuryTrade::UserService>();
//     auto marketDataService = std::make_shared<mercuryTrade::MarketDataService>();
//     auto orderBookService = std::make_shared<mercuryTrade::OrderBookService>();
//     auto orderService = std::make_shared<mercuryTrade::OrderService>();


//     auto authController = std::make_shared<mercuryTrade::api::auth::AuthController>(userService);
//     auto marketDataController = std::make_shared<mercuryTrade::api::market::MarketDataController>(
//         marketDataService, orderBookService);
//     auto orderController = std::make_shared<mercuryTrade::api::orders::OrderController>(orderService);

//     mercuryTrade::http::Server server(3000);


//     server.post("/api/auth/login", [&](const mercuryTrade::http::Request& req) { 
//         return authController->login(req); 
//     });
//     server.post("/api/auth/register", [&](const mercuryTrade::http::Request& req) { 
//         return authController->registerUser(req); 
//     });
//     server.post("/api/auth/logout", [&](const mercuryTrade::http::Request& req) { 
//         return authController->logout(req); 
//     });


//     server.get("/api/market-data/{symbol}", [&](const mercuryTrade::http::Request& req) { 
//         return marketDataController->getMarketData(req.getParam("symbol")); 
//     });
    
//     server.get("/api/order-book/{symbol}", [&](const mercuryTrade::http::Request& req) { 
//         return marketDataController->getOrderBook(req.getParam("symbol")); 
//     });


//     server.get("/api/orders", [&](const mercuryTrade::http::Request& req) { 
//         return orderController->getOrders(); 
//     });
    
//     server.get("/api/orders/{id}", [&](const mercuryTrade::http::Request& req) { 
//         return orderController->getOrderById(req.getParam("id")); 
//     });
    
//     server.post("/api/orders", [&](const mercuryTrade::http::Request& req) { 
//         return orderController->placeOrder(req); 
//     });

//     server.start();
//     return 0;
// }