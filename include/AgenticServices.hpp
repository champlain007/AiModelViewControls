#pragma once

#include "IAgenticService.hpp"
#include "cli/CliClient.hpp"
#include "docker/DockerFacade.hpp"
#include "Orchestrator.hpp"
#include "McpServer.hpp"
#include "WebSocketSandbox.hpp"
#include "Controller.hpp"
#include "Model.hpp"
#include "ViewTUI.hpp"
#include "connectors/ConnectorStrategies.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>

class CliService : public IAgenticService {
public:
    CliService(const std::string& host, int port, const std::vector<std::string>& args)
        : m_client(host, port), m_args(args) {}
    int run() override {
        return m_client.executeCommand(m_args);
    }
private:
    AgenticCliClient m_client;
    std::vector<std::string> m_args;
};

class DockerService : public IAgenticService {
public:
    DockerService(const std::string& host, int port, const std::vector<std::string>& args)
        : m_client(host, port), m_args(args) {}
    int run() override {
        return m_client.execute(m_args);
    }
private:
    DockerFacade m_client;
    std::vector<std::string> m_args;
};

class OrchestratorService : public IAgenticService {
public:
    OrchestratorService(int port) : m_port(port) {}
    int run() override {
        AgenticOrchestratorController orchestrator;
        orchestrator.start(m_port);
        return 0;
    }
private:
    int m_port;
};

class McpService : public IAgenticService {
public:
    McpService(int port) : m_port(port) {}
    int run() override {
        AgenticMcpController mcp;
        mcp.start(m_port);
        return 0;
    }
private:
    int m_port;
};

class WebSocketSandboxService : public IAgenticService {
public:
    WebSocketSandboxService(int port) : m_port(port) {}
    int run() override {
        AgenticWebSocketController ws;
        ws.start(m_port);
        return 0;
    }
private:
    int m_port;
};

class MainService : public IAgenticService {
public:
    MainService(int port, bool headless, bool enableSentry)
        : m_port(port), m_headless(headless), m_enableSentry(enableSentry) {}
    
    int run() override {
        AgenticPipelineModel model;
        AgenticHttpController controller(model, m_port);
        controller.start();

        if (m_enableSentry) {
            std::filesystem::path sentryPath = std::filesystem::current_path() / "privacy_sentry.log";
            std::string sentryLog = sentryPath.string();
            AgentHubConnector sentryConn;
            sentryConn.id = "PrivacySentry_Agent";
            sentryConn.strategy = std::make_unique<LocalFileStrategy>();
            sentryConn.target = sentryLog;
            sentryConn.metadata = {{"role", "security_hids"}, {"owner", "PrivacySentry"}};
            model.addConnector(sentryConn);
            model.triggerAlert("INTEGRATION_ACTIVE", "AgenticCore", "PrivacySentry log tailing enabled via startup flag.");
        }

        if (m_headless) {
            std::cout << "[AGENT_SYSTEM] Agent Node started in HEADLESS mode on port " << m_port << std::endl;
            std::cout << "[AGENT_SYSTEM] Press Ctrl+C to terminate." << std::endl;
            while (true) { std::this_thread::sleep_for(std::chrono::seconds(1)); }
        } else {
            AgenticDashboardView dashboard(model);
            dashboard.run();
        }

        controller.stop();
        return 0;
    }
private:
    int m_port;
    bool m_headless;
    bool m_enableSentry;
};
