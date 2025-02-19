// src/http/Server.cpp
#include "mercuryTrade/http/Server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <regex>

namespace mercuryTrade {
namespace http {

Server::Server(int port) : port_(port) {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("Failed to set socket options");
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        throw std::runtime_error("Failed to bind to port");
    }
}

void Server::start() {
    if (listen(server_fd_, 3) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }

    std::cout << "Server listening on port " << port_ << std::endl;

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        // Handle each connection in a new thread
        std::thread([this, client_fd]() {
            handle_connection(client_fd);
        }).detach();
    }
}

void Server::handle_connection(int client_fd) {
    char buffer[4096] = {0};
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
    
    if (bytes_read > 0) {
        Request req = parse_request(std::string(buffer, bytes_read));
        Response res;

        // Handle CORS preflight
        if (req.method == "OPTIONS") {
            res.headers["Access-Control-Allow-Origin"] = "*";
            res.headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
            res.headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
            res.status = 204;
        } else {
            // Find and execute route handler
            std::string route_key = req.method + " " + req.path;
            auto handler = route_handlers_.find(route_key);
            
            if (handler != route_handlers_.end()) {
                try {
                    res = handler->second(req);
                } catch (const std::exception& e) {
                    res = Response::json({{"error", e.what()}}, 500);
                }
            } else {
                res = Response::json({{"error", "Not Found"}}, 404);
            }
        }

        // Add CORS headers to all responses
        res.headers["Access-Control-Allow-Origin"] = "*";
        
        send_response(client_fd, res);
    }

    close(client_fd);
}

Request Server::parse_request(const std::string& raw_request) {
    Request req;
    std::istringstream stream(raw_request);
    std::string line;

    // Parse request line
    std::getline(stream, line);
    std::istringstream request_line(line);
    request_line >> req.method >> req.path;

    // Extract route parameters
    for (const auto& [route_pattern, _] : route_handlers_) {
        std::regex pattern(route_pattern_to_regex(route_pattern));
        std::smatch matches;
        if (std::regex_match(req.path, matches, pattern)) {
            std::vector<std::string> param_names = extract_param_names(route_pattern);
            for (size_t i = 1; i < matches.size() && i-1 < param_names.size(); ++i) {
                req.params[param_names[i-1]] = matches[i].str();
            }
            break;
        }
    }

    // Parse headers
    while (std::getline(stream, line) && line != "\r") {
        auto separator = line.find(':');
        if (separator != std::string::npos) {
            std::string key = line.substr(0, separator);
            std::string value = line.substr(separator + 2, line.length() - separator - 3);
            req.headers[key] = value;
        }
    }

    // Read body
    std::stringstream body_stream;
    while (std::getline(stream, line)) {
        body_stream << line << "\n";
    }
    req.body = body_stream.str();

    return req;
}

void Server::send_response(int client_fd, const Response& res) {
    std::stringstream response_stream;
    response_stream << "HTTP/1.1 " << res.status << " " << get_status_text(res.status) << "\r\n";
    
    // Add content type if not present
    if (res.headers.find("Content-Type") == res.headers.end()) {
        response_stream << "Content-Type: application/json\r\n";
    }

    // Add other headers
    for (const auto& [key, value] : res.headers) {
        response_stream << key << ": " << value << "\r\n";
    }

    // Add content length and body
    response_stream << "Content-Length: " << res.body.length() << "\r\n\r\n";
    response_stream << res.body;

    std::string response_str = response_stream.str();
    send(client_fd, response_str.c_str(), response_str.length(), 0);
}

std::string Server::route_pattern_to_regex(const std::string& pattern) {
    std::string regex_str = pattern;
    std::regex param_regex("\\{([^}]+)\\}");
    regex_str = std::regex_replace(regex_str, param_regex, "([^/]+)");
    return "^" + regex_str + "$";
}

std::vector<std::string> Server::extract_param_names(const std::string& pattern) {
    std::vector<std::string> names;
    std::regex param_regex("\\{([^}]+)\\}");
    auto param_begin = std::sregex_iterator(pattern.begin(), pattern.end(), param_regex);
    auto param_end = std::sregex_iterator();

    for (auto it = param_begin; it != param_end; ++it) {
        names.push_back((*it)[1].str());
    }
    return names;
}

std::string Server::get_status_text(int status) {
    switch (status) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default: return "Unknown";
    }
}

void Server::register_route(const std::string& method, const std::string& path, RequestHandler handler) {
    route_handlers_[method + " " + path] = std::move(handler);
}

void Server::get(const std::string& path, RequestHandler handler) {
    register_route("GET", path, std::move(handler));
}

void Server::post(const std::string& path, RequestHandler handler) {
    register_route("POST", path, std::move(handler));
}

void Server::put(const std::string& path, RequestHandler handler) {
    register_route("PUT", path, std::move(handler));
}

void Server::del(const std::string& path, RequestHandler handler) {
    register_route("DELETE", path, std::move(handler));
}

}} // namespace mercuryTrade::http