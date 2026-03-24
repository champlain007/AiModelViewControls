#pragma once
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "Model.hpp"
#include "XWayland.hpp"

class AgenticOrchestratorController {
public:
    AgenticOrchestratorController();
    ~AgenticOrchestratorController();
    void start(int port);
    void stop();

private:
    void maintenanceLoop();
    void setupRoutes();
    void loadNodes();
    void saveNodes();
    void enforceSecurity(const std::string& url, const std::string& agentId);
    void ensurePipe(const std::string& source, const std::string& target);
    void ensureTestAgent();

    httplib::Server m_svr;
    std::thread m_maintenanceThread;
    std::atomic<bool> m_running;
    nlohmann::json m_nodes;
    std::mutex m_nodesMutex;
    std::string m_scriptDir;
    std::string m_binaryPath;
    XWayland m_wayland;
};
