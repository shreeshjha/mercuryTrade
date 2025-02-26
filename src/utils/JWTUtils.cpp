#include "mercuryTrade/utils/JWTUtils.hpp"
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <stdexcept>

namespace mercuryTrade {
namespace utils {

std::string JWTUtils::generateToken(const nlohmann::json& payload, const std::string& secret, int expiry_hours) {
    // Create header
    nlohmann::json header = {
        {"alg", "HS256"},
        {"typ", "JWT"}
    };
    
    // Add expiry to payload if not present
    nlohmann::json claims = payload;
    if (!claims.contains("exp")) {
        auto now = std::chrono::system_clock::now();
        auto exp_time = now + std::chrono::hours(expiry_hours);
        auto exp_time_t = std::chrono::system_clock::to_time_t(exp_time);
        claims["exp"] = static_cast<unsigned long>(exp_time_t);
    }
    
    if (!claims.contains("iat")) {
        auto now = std::chrono::system_clock::now();
        auto now_t = std::chrono::system_clock::to_time_t(now);
        claims["iat"] = static_cast<unsigned long>(now_t);
    }
    
    // Encode header and payload
    std::string header_encoded = base64UrlEncode(header.dump());
    std::string payload_encoded = base64UrlEncode(claims.dump());
    
    // Create signature
    std::string signature_input = header_encoded + "." + payload_encoded;
    std::string signature = hmacSha256(signature_input, secret);
    std::string signature_encoded = base64UrlEncode(signature);
    
    // Combine to form JWT
    return header_encoded + "." + payload_encoded + "." + signature_encoded;
}

nlohmann::json JWTUtils::verifyToken(const std::string& token, const std::string& secret) {
    // Split token into parts
    auto first_dot = token.find('.');
    auto last_dot = token.rfind('.');
    
    if (first_dot == std::string::npos || last_dot == std::string::npos || first_dot == last_dot) {
        throw std::runtime_error("Invalid token format");
    }
    
    std::string header_encoded = token.substr(0, first_dot);
    std::string payload_encoded = token.substr(first_dot + 1, last_dot - first_dot - 1);
    std::string signature_encoded = token.substr(last_dot + 1);
    
    // Verify signature
    std::string signature_input = header_encoded + "." + payload_encoded;
    std::string computed_signature = hmacSha256(signature_input, secret);
    std::string computed_signature_encoded = base64UrlEncode(computed_signature);
    
    if (computed_signature_encoded != signature_encoded) {
        throw std::runtime_error("Invalid token signature");
    }
    
    // Decode payload
    std::string payload_decoded = base64UrlDecode(payload_encoded);
    nlohmann::json claims = nlohmann::json::parse(payload_decoded);
    
    // Check expiry
    if (claims.contains("exp")) {
        auto now = std::chrono::system_clock::now();
        auto now_t = std::chrono::system_clock::to_time_t(now);
        
        unsigned long exp = claims["exp"].get<unsigned long>();
        if (static_cast<unsigned long>(now_t) > exp) {
            throw std::runtime_error("Token expired");
        }
    }
    
    return claims;
}

std::string JWTUtils::base64UrlEncode(const std::string& input) {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
        
    std::string ret;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    int i = 0;
    int j = 0;
    unsigned char array_3_len = 0;
    
    for (char c : input) {
        char_array_3[i++] = c;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (i = 0; i < 4; i++) {
                ret += base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }
    
    if (i) {
        array_3_len = i;
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (j = 0; j < array_3_len + 1; j++) {
            ret += base64_chars[char_array_4[j]];
        }
        
        while (array_3_len++ < 3) {
            ret += '=';
        }
    }
    
    // URL safe: replace '+' with '-', '/' with '_', and remove '='
    std::string url_safe = ret;
    std::replace(url_safe.begin(), url_safe.end(), '+', '-');
    std::replace(url_safe.begin(), url_safe.end(), '/', '_');
    url_safe.erase(std::remove(url_safe.begin(), url_safe.end(), '='), url_safe.end());
    
    return url_safe;
}

std::string JWTUtils::base64UrlDecode(const std::string& input) {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    // URL safe: replace '-' with '+', '_' with '/'
    std::string standard_base64 = input;
    std::replace(standard_base64.begin(), standard_base64.end(), '-', '+');
    std::replace(standard_base64.begin(), standard_base64.end(), '_', '/');
    
    // Add padding if needed
    int padding = 4 - (standard_base64.length() % 4);
    if (padding < 4) {
        standard_base64.append(padding, '=');
    }
    
    std::string ret;
    int i = 0;
    unsigned char char_array_4[4], char_array_3[3];
    
    for (char c : standard_base64) {
        if (c == '=') {
            break;
        }
        
        auto pos = base64_chars.find(c);
        if (pos == std::string::npos) {
            continue; // Skip invalid characters
        }
        
        char_array_4[i++] = pos;
        if (i == 4) {
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            
            for (i = 0; i < 3; i++) {
                ret += char_array_3[i];
            }
            i = 0;
        }
    }
    
    if (i) {
        for (int j = i; j < 4; j++) {
            char_array_4[j] = 0;
        }
        
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        
        for (int j = 0; j < i - 1; j++) {
            ret += char_array_3[j];
        }
    }
    
    return ret;
}

std::string JWTUtils::hmacSha256(const std::string& data, const std::string& key) {
    unsigned char* digest = HMAC(EVP_sha256(), 
                                key.c_str(), key.length(),
                                reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
                                NULL, NULL);
    
    std::ostringstream oss;
    for (int i = 0; i < 32; ++i) { // SHA-256 produces 32 bytes
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    
    return oss.str();
}
}}