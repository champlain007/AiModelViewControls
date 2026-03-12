#include "McpServer.hpp"
#include "Security.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

void AgenticMcpController::start(int port) {
    m_svr.Post("/mcp/inference", [](const httplib::Request& req, httplib::Response& res) {
        std::string auth = req.get_header_value("Authorization");
        std::string token = (auth.find("Bearer ") == 0) ? auth.substr(7) : "";

        auto data = nlohmann::json::parse(req.body);
        std::string agentId = data.value("agent_id", "unknown");
        std::string prompt = data.value("prompt", "");

        nlohmann::json result;
        if (prompt.find("check files") != std::string::npos) {
#ifdef _WIN32
            std::string sensitiveFile = "C:\\Windows\\System32\\config\\SAM";
#else
            std::string sensitiveFile = "/etc/shadow";
#endif
            result = security::AgentSandbox::executeTool("read_file", {{"path", sensitiveFile}}, agentId, token);
        } else {
            result = {{"response", "Agentic inference processed within secure C++ sandbox."}, {"status", "OK"}};
        }
        res.set_content(result.dump(), "application/json");
    });

    m_svr.Post("/mcp/override", [](const httplib::Request& req, httplib::Response& res) {
        auto data = nlohmann::json::parse(req.body);
        security::AgentSandbox::updatePolicy(data.value("tool", ""), data.value("action", ""));
        res.set_content("Agent Policy Updated", "text/plain");
    });

    std::cout << "[MCP_GATEWAY] Agentic Inference Server active on port " << port << std::endl;
    m_svr.listen("0.0.0.0", port);
}

void AgenticMcpController::stop() {
    m_svr.stop();
}
