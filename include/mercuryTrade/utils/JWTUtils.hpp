#pragma once

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

namespace mercuryTrade {
namespace utils {

class JWTUtils {
public:
    // Generate a JWT token with the given payload and secret
    static std::string generateToken(const nlohmann::json& payload, const std::string& secret, int expiry_hours = 24);
    
    // Verify and decode a JWT token
    static nlohmann::json verifyToken(const std::string& token, const std::string& secret);
    
private:
    // Base64 encoding/decoding utilities
    static std::string base64UrlEncode(const std::string& input);
    static std::string base64UrlDecode(const std::string& input);
    
    // HMAC-SHA256 signing
    static std::string hmacSha256(const std::string& data, const std::string& key);
};

}} // namespace mercuryTrade::utils


