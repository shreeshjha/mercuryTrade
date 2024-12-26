#pragma once
#include <string>
#include <functional>
#include <map>
#include <nlohmann/json.hpp>

namespace mercuryTrade {
namespace http {

class Request {
public:
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> params;

    std::string getParam(const std::string& name, const std::string& defaultValue = "") const {
        auto it = params.find(name);
        return it != params.end() ? it->second : defaultValue;
    }
};

class Response {
public:
    int status = 200;
    std::string body;
    std::map<std::string, std::string> headers;
    
    static Response json(const nlohmann::json& data, int status = 200) {
        Response res;
        res.status = status;
        res.body = data.dump();
        res.headers["Content-Type"] = "application/json";
        return res;
    }
};

using RequestHandler = std::function<Response(const Request&)>;

class Server {
public:
    Server(int port = 3000);
    void start();
    
    void get(const std::string& path, RequestHandler handler);
    void post(const std::string& path, RequestHandler handler);
    void put(const std::string& path, RequestHandler handler);
    void del(const std::string& path, RequestHandler handler);

private:
    int port_;
    int server_fd_;
    std::map<std::string, RequestHandler> route_handlers_;
    
    void handle_connection(int client_fd);
    Request parse_request(const std::string& raw_request);
    void send_response(int client_fd, const Response& res);
    std::string route_pattern_to_regex(const std::string& pattern);
    std::vector<std::string> extract_param_names(const std::string& pattern);
    std::string get_status_text(int status);
    void register_route(const std::string& method, const std::string& path, RequestHandler handler);
};

}} // namespace