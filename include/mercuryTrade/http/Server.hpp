#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>

namespace mercuryTrade {
namespace http {

// Forward declarations
struct Request;
struct Response;

using RequestHandler = std::function<Response(const Request&)>;

struct Request {
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> params;
    std::unordered_map<std::string, std::string> query_params;
    std::string body;
    nlohmann::json json_body;
    std::string user_id;   // Set if JWT token is valid
    std::string user_role; // Role from JWT token
};

struct Response {
    int status = 200;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    static Response json(const nlohmann::json& data, int status = 200) {
        Response res;
        res.status = status;
        res.headers["Content-Type"] = "application/json";
        res.body = data.dump();
        return res;
    }
};

class Server {
public:
    Server(int port, const std::string& jwt_secret = "");
    
    void start();
    
    void get(const std::string& path, RequestHandler handler);
    void post(const std::string& path, RequestHandler handler);
    void put(const std::string& path, RequestHandler handler);
    void del(const std::string& path, RequestHandler handler);
    
    // Add middleware support
    void use_auth_middleware(const std::string& path_prefix);
    
private:
    void register_route(const std::string& method, const std::string& path, RequestHandler handler);
    void handle_connection(int client_fd);
    Request parse_request(const std::string& raw_request);
    void send_response(int client_fd, const Response& res);
    
    std::string route_pattern_to_regex(const std::string& pattern);
    std::vector<std::string> extract_param_names(const std::string& pattern);
    std::string get_status_text(int status);
    bool is_auth_required(const std::string& path) const;
    
    // URL encoding/decoding utilities
    std::string url_encode(const std::string& value);
    std::string url_decode(const std::string& value);
    
    int server_fd_;
    int port_;
    std::string jwt_secret_;
    std::unordered_map<std::string, RequestHandler> route_handlers_;
    std::vector<std::string> auth_paths_;
    std::set<std::string> allowed_origins_;
};

}} // namespace mercuryTrade::http
// #pragma once
// #include <string>
// #include <functional>
// #include <map>
// #include <nlohmann/json.hpp>

// namespace mercuryTrade {
// namespace http {

// class Request {
// public:
//     std::string method;
//     std::string path;
//     std::string body;
//     std::map<std::string, std::string> headers;
//     std::map<std::string, std::string> params;

//     std::string getParam(const std::string& name, const std::string& defaultValue = "") const {
//         auto it = params.find(name);
//         return it != params.end() ? it->second : defaultValue;
//     }
// };

// class Response {
// public:
//     int status = 200;
//     std::string body;
//     std::map<std::string, std::string> headers;
    
//     static Response json(const nlohmann::json& data, int status = 200) {
//         Response res;
//         res.status = status;
//         res.body = data.dump();
//         res.headers["Content-Type"] = "application/json";
//         return res;
//     }
// };

// using RequestHandler = std::function<Response(const Request&)>;

// class Server {
// public:
//     Server(int port = 3000);
//     void start();
    
//     void get(const std::string& path, RequestHandler handler);
//     void post(const std::string& path, RequestHandler handler);
//     void put(const std::string& path, RequestHandler handler);
//     void del(const std::string& path, RequestHandler handler);

// private:
//     int port_;
//     int server_fd_;
//     std::map<std::string, RequestHandler> route_handlers_;
    
//     void handle_connection(int client_fd);
//     Request parse_request(const std::string& raw_request);
//     void send_response(int client_fd, const Response& res);
//     std::string route_pattern_to_regex(const std::string& pattern);
//     std::vector<std::string> extract_param_names(const std::string& pattern);
//     std::string get_status_text(int status);
//     void register_route(const std::string& method, const std::string& path, RequestHandler handler);
// };

// }} // namespace