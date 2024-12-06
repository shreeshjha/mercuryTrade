// include/mercuryTrade/api/auth/AuthController.hpp
#pragma once

#include <memory>
#include <crow.h>
#include "../../services/UserService.hpp"

namespace mercuryTrade {
namespace api {
namespace auth {

class AuthController {
public:
    explicit AuthController(std::shared_ptr<UserService> userService);

    crow::response login(const crow::request& req);
    crow::response registerUser(const crow::request& req);
    crow::response logout(const crow::request& req);

private:
    std::shared_ptr<UserService> m_userService;
};
