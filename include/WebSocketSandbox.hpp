#pragma once
#include <ixwebsocket/IXWebSocketServer.h>
#include <string>
#include <vector>
#include <memory>
#include "XWayland.hpp"

class AgenticWebSocketController {
public:
    void start(int port);
    void stop();

private:
    std::unique_ptr<ix::WebSocketServer> m_server;
    std::vector<std::string> m_allowedActions = {"subscribe", "heartbeat", "query_status"};
    XWayland m_wayland;
};
