// include/mercuryTrade/websocket/WebSocketServer.hpp
#pragma once
#include <string>
#include <functional>
#include <map>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

namespace mercuryTrade {
namespace websocket {

class WebSocketServer {
public:
    WebSocketServer(int port);
    void start();
    void broadcast(const std::string& channel, const std::string& message);

private:
    using WsServer = websocketpp::server<websocketpp::config::asio>;
    WsServer server_;
    std::map<std::string, std::vector<websocketpp::connection_hdl>> subscriptions_;

    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, WsServer::message_ptr msg);
};

}} // namespace
