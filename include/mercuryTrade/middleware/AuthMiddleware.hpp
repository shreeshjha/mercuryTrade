// include/mercuryTrade/middleware/AuthMiddleware.hpp
#pragma once
#include "mercuryTrade/http/Server.hpp"
#include <jwt-cpp/jwt.h>

namespace mercuryTrade {
namespace middleware {

class AuthMiddleware {
public:
    static http::Response authenticate(const http::Request& req) {
        auto authHeader = req.headers.find("Authorization");
        if (authHeader == req.headers.end()) {
            return http::Response::json({{"error", "No authorization token"}}, 401);
        }

        try {
            // Extract token from "Bearer <token>"
            std::string token = authHeader->second.substr(7);
            auto decoded = jwt::decode(token);
            
            // Verify token
            auto verifier = jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256("6fh26cj27xn72nf7n27fndsknfw7"));


            
            verifier.verify(decoded);
            return http::Response::json({{"status", "success"}}, 200);
        } catch (const std::exception& e) {
            return http::Response::json({{"error", "Invalid token"}}, 401);
        }
    }
};

}} // namespace
