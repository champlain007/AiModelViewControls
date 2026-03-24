#include "WebSocketSandbox.hpp"
#include "Security.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

void AgenticWebSocketController::start(int port) {
    m_wayland.start();
    m_server = std::make_unique<ix::WebSocketServer>(port, "0.0.0.0");

    m_server->setOnClientMessageCallback([this](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            std::cout << "[AGENT_WS_SANDBOX] Secure connection established for agent session: " << connectionState->getId() << std::endl;
        } else if (msg->type == ix::WebSocketMessageType::Message) {
            try {
                auto data = nlohmann::json::parse(msg->str);
                std::string action = data.value("action", "");

                bool allowed = false;
                for (const auto& a : m_allowedActions) {
                    if (action == a) { allowed = true; break; }
                }

                if (!allowed) {
                    std::cout << "[AGENT_WS_SECURITY] Blocked unauthorized agent action '" << action << "'" << std::endl;
                    webSocket.send("{\"status\": \"AGENT_SANDBOX_VIOLATION\"}");
                } else {
                    std::cout << "[AGENT_WS_SANDBOX] Authorized action: " << action << std::endl;
                    webSocket.send("{\"status\": \"OK\", \"action\": \"" + action + "\"}");
                }
            } catch (...) {
                std::cout << "[AGENT_WS_SECURITY] Dropped malformed agent communication frame" << std::endl;
            }
        }
    });

    auto res = m_server->listen();
    if (!res.first) {
        std::cerr << "[AGENT_WS_SANDBOX] Failed to initialize agentic websocket server: " << res.second << std::endl;
        return;
    }

    std::cout << "[AGENT_WS_SANDBOX] Agentic Communication Sandbox active on port " << port << std::endl;
    m_server->start();
}

void AgenticWebSocketController::stop() {
    if (m_server) m_server->stop();
    m_wayland.stop();
}
