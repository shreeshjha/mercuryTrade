// src/api/auth/AuthController.cpp
#include "../../../include/mercuryTrade/api/auth/AuthController.hpp"
#include <nlohmann/json.hpp>

namespace mercuryTrade {
namespace api {
namespace auth {

AuthController::AuthController(std::shared_ptr<UserService> userService) 
    : m_userService(userService) {}

http::Response AuthController::login(const http::Request& req) {
    try {
        auto data = nlohmann::json::parse(req.body);
        auto user = m_userService->authenticate(
            data["email"].get<std::string>(), 
            data["password"].get<std::string>()
        );
        
        if (!user) {
            return http::Response::json({{"error", "Invalid credentials"}}, 401);
        }

        return http::Response::json(user->toJson());
    } catch (const std::exception& e) {
        return http::Response::json({{"error", e.what()}}, 400);
    }
}

http::Response AuthController::registerUser(const http::Request& req) {
    try {
        auto data = nlohmann::json::parse(req.body);
        auto user = m_userService->createUser(
            data["email"].get<std::string>(),
            data["username"].get<std::string>(),
            data["password"].get<std::string>()
        );
        
        return http::Response::json(user.toJson(), 201);
    } catch (const std::exception& e) {
        return http::Response::json({{"error", e.what()}}, 400);
    }
}

http::Response AuthController::logout(const http::Request& req) {
    return http::Response::json({{"status", "success"}});
}

}}} // namespace