// include/mercuryTrade/api/auth/AuthController.hpp
#pragma once
#include "../../http/Server.hpp"
#include "../../services/UserService.hpp"
#include <memory>

namespace mercuryTrade {
namespace api {
namespace auth {

class AuthController {
public:
    explicit AuthController(std::shared_ptr<UserService> userService);
    
    http::Response login(const http::Request& req);
    http::Response registerUser(const http::Request& req);
    http::Response logout(const http::Request& req);

private:
    std::shared_ptr<UserService> m_userService;
};

}}}