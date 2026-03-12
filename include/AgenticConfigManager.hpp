#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

/**
 * @brief Builder for the tradesecret.sh script content.
 * Follows the Builder Design Pattern.
 */
class TradeSecretBuilder {
public:
    TradeSecretBuilder() {
        m_content = "#!/bin/bash\n";
        m_content += "# ==============================================================================\n";
        m_content += "# AgenticMVCpipe - Localized Trade Secret Configuration and Boot Script\n";
        m_content += "# ==============================================================================\n";
    }

    TradeSecretBuilder& setHandshakeKey(const std::string& key) {
        m_handshakeKey = key;
        return *this;
    }

    TradeSecretBuilder& setPorts(int orch, int mcp, int ws, int http) {
        m_orchPort = orch;
        m_mcpPort = mcp;
        m_wsPort = ws;
        m_httpPort = http;
        return *this;
    }

    TradeSecretBuilder& setSandboxDir(const std::string& dir) {
        m_sandboxDir = dir;
        return *this;
    }

    std::string build() {
        std::string res = m_content;
        res += "export TRADESECRET_HANDSHAKE_KEY=\"" + m_handshakeKey + "\"\n";
        res += "export ORCH_PORT=${ORCH_PORT:-" + std::to_string(m_orchPort) + "}\n";
        res += "export MCP_PORT=${MCP_PORT:-" + std::to_string(m_mcpPort) + "}\n";
        res += "export WS_PORT=${WS_PORT:-" + std::to_string(m_wsPort) + "}\n";
        res += "export PORT=${PORT:-" + std::to_string(m_httpPort) + "}\n";
        res += "export AGENT_SANDBOX_DIR=\"" + m_sandboxDir + "\"\n";
        
        res += "BINARY=\"./build/AgenticPipeline\"\n";
        res += "if [ ! -f \"$BINARY\" ]; then mkdir -p build && cd build && cmake .. && make && cd ..; fi\n";
        
        res += "if [ \"$1\" == \"--interactive\" ] || [ \"$1\" == \"-i\" ]; then\n";
        res += "    read -p \"Enter Orchestrator Port [$ORCH_PORT]: \" user_orch\n";
        res += "    export ORCH_PORT=${user_orch:-$ORCH_PORT}\n";
        res += "    read -p \"Enter MCP Gateway Port [$MCP_PORT]: \" user_mcp\n";
        res += "    export MCP_PORT=${user_mcp:-$MCP_PORT}\n";
        res += "    read -p \"Enter WebSocket Sandbox Port [$WS_PORT]: \" user_ws\n";
        res += "    export WS_PORT=${user_ws:-$WS_PORT}\n";
        res += "    read -p \"Enter Dashboard HTTP Port [$PORT]: \" user_port\n";
        res += "    export PORT=${user_port:-$PORT}\n";
        res += "fi\n";

        res += "$BINARY --orchestrator &\nORCH_PID=$!\n";
        res += "$BINARY --mcp &\nMCP_PID=$!\n";
        res += "$BINARY --ws-sandbox &\nWS_PID=$!\n";
        res += "trap 'kill $ORCH_PID $MCP_PID $WS_PID 2>/dev/null; exit 0' SIGINT SIGTERM EXIT\n";
        res += "$BINARY $HEADLESS_FLAG\n";
        
        return res;
    }

private:
    std::string m_content;
    std::string m_handshakeKey = "APPROVED_BY_TRADESECRET";
    std::string m_sandboxDir = ".";
    int m_orchPort = 9000;
    int m_mcpPort = 9100;
    int m_wsPort = 9200;
    int m_httpPort = 8080;
};

/**
 * @brief Singleton Manager for Agentic Configuration and Sandboxing.
 * Follows the Singleton Design Pattern.
 */
class AgenticConfigManager {
public:
    static AgenticConfigManager& getInstance() {
        static AgenticConfigManager instance;
        return instance;
    }

    void initializeSandbox(const std::string& instanceId = "") {
        if (!instanceId.empty()) {
            m_sandboxPath = std::filesystem::current_path() / "sandboxes" / instanceId;
        } else {
            const char* envSandbox = std::getenv("AGENT_SANDBOX_DIR");
            if (envSandbox) {
                m_sandboxPath = envSandbox;
            } else {
                m_sandboxPath = std::filesystem::current_path();
            }
        }

        std::error_code ec;
        if (!std::filesystem::exists(m_sandboxPath, ec)) {
            std::filesystem::create_directories(m_sandboxPath, ec);
        }
    }

    void ensureLocalizedTradeSecret() {
        std::filesystem::path tsPath = m_sandboxPath / "tradesecret.sh";
        
        std::error_code ec;
        if (!std::filesystem::exists(tsPath, ec) || std::filesystem::file_size(tsPath, ec) == 0) {
            TradeSecretBuilder builder;
            builder.setSandboxDir(m_sandboxPath.string());
            
            // In a real scenario, we might pull these from a central config or env
            // For now, we use defaults in the builder.

            std::ofstream f(tsPath);
            if (f.is_open()) {
                f << builder.build();
                f.close();
                std::filesystem::permissions(tsPath, std::filesystem::perms::owner_all);
                std::cerr << "[SANDBOX] Localized tradesecret.sh generated in: " << m_sandboxPath << std::endl;
            }
        }
    }

    std::filesystem::path getSandboxPath() const { return m_sandboxPath; }

private:
    AgenticConfigManager() : m_sandboxPath(std::filesystem::current_path()) {}
    ~AgenticConfigManager() = default;
    AgenticConfigManager(const AgenticConfigManager&) = delete;
    AgenticConfigManager& operator=(const AgenticConfigManager&) = delete;

    std::filesystem::path m_sandboxPath;
};
