// include/mercuryTrade/config/Config.hpp
#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace mercuryTrade {
namespace config {

class Config {
public:
    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    void load(const std::string& configPath);

    std::string getDatabaseUrl() const { return dbUrl_; }
    std::string getJwtSecret() const { return jwtSecret_; }
    int getServerPort() const { return serverPort_; }
    std::string getWebSocketUrl() const { return wsUrl_; }
    
private:
    Config() = default;
    
    std::string dbUrl_;
    std::string jwtSecret_;
    int serverPort_ = 3000;
    std::string wsUrl_;
};

}} // namespace
