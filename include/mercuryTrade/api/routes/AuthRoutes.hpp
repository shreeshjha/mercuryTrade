#pragma once

#include <crow.h>
#include "../auth/AuthController.hpp"

namespace mercuryTrade {
namespace api {
namespace routes {

void registerAuthRoutes(crow::App<crow::CORSHandler>& app, 
    std::shared_ptr<auth::AuthController> authController);

}}}
