// include/mercuryTrade/utils/Logger.hpp
#pragma once
#include <spdlog/spdlog.h>

namespace mercuryTrade {
namespace utils {

class Logger {
public:
    static void init();
    static std::shared_ptr<spdlog::logger> get(const std::string& name);

private:
    static std::map<std::string, std::shared_ptr<spdlog::logger>> loggers_;
};

}} // namespace
