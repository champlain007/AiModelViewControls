#pragma once

#include "IAgenticService.hpp"
#include "AgenticServices.hpp"
#include <memory>
#include <string>
#include <vector>
#include <cstdlib>

class AgenticServiceFactory {
public:
    static std::unique_ptr<IAgenticService> createService(int argc, char** argv) {
        if (argc >= 2) {
            std::string mode = argv[1];
            std::vector<std::string> subArgs;
            for (int i = 2; i < argc; ++i) subArgs.push_back(argv[i]);

            int port = 8080;
            if (std::getenv("PORT")) {
                try { port = std::stoi(std::getenv("PORT")); } catch (...) {}
            }

            if (mode == "--cli") {
                return std::make_unique<CliService>("127.0.0.1", port, subArgs);
            }
            if (mode == "--docker") {
                return std::make_unique<DockerService>("127.0.0.1", port, subArgs);
            }
        }

        bool enableSentry = false;
        bool headless = false;
        bool isOrchestrator = false;
        bool isMcp = false;
        bool isWsSandbox = false;

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--sentry") enableSentry = true;
            if (arg == "--headless") headless = true;
            if (arg == "--orchestrator") isOrchestrator = true;
            if (arg == "--mcp") isMcp = true;
            if (arg == "--ws-sandbox") isWsSandbox = true;
        }

        int orchPort = 9000;
        int mcpPort = 9100;
        int wsPort = 9200;
        int port = 8080;

        try { if (std::getenv("ORCH_PORT")) orchPort = std::stoi(std::getenv("ORCH_PORT")); } catch (...) {}
        try { if (std::getenv("MCP_PORT")) mcpPort = std::stoi(std::getenv("MCP_PORT")); } catch (...) {}
        try { if (std::getenv("WS_PORT")) wsPort = std::stoi(std::getenv("WS_PORT")); } catch (...) {}
        try { if (std::getenv("PORT")) port = std::stoi(std::getenv("PORT")); } catch (...) {}

        if (isOrchestrator) {
            return std::make_unique<OrchestratorService>(orchPort);
        }
        if (isMcp) {
            return std::make_unique<McpService>(mcpPort);
        }
        if (isWsSandbox) {
            return std::make_unique<WebSocketSandboxService>(wsPort);
        }

        return std::make_unique<MainService>(port, headless, enableSentry);
    }

    static bool requiresTradeSecret(int argc, char** argv) {
        if (argc >= 2) {
            std::string mode = argv[1];
            if (mode == "--cli" || mode == "--docker") return false;
        }
        return true;
    }
};
