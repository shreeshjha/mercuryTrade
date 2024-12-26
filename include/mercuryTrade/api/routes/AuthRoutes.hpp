#pragma once

#include <crow.h>
#include "../auth/AuthController.hpp"

namespace mercuryTrade {
namespace api {
namespace routes {

void registerAuthRoutes(crow::SimpleApp& app, std::shared_ptr<auth::AuthController> controller) {
    CROW_ROUTE(app, "/api/auth/login").methods(crow::HTTPMethod::POST)
    ([controller](const crow::request& req) {
        return controller->login(req);
    });

    CROW_ROUTE(app, "/api/auth/register").methods(crow::HTTPMethod::POST)
    ([controller](const crow::request& req) {
        return controller->registerUser(req);
    });

    CROW_ROUTE(app, "/api/auth/logout").methods(crow::HTTPMethod::POST)
    ([controller](const crow::request& req) {
        return controller->logout(req);
    });
}

}}} // namespace