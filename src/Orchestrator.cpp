#include "Orchestrator.hpp"
#include "Security.hpp"
#include "AgenticConfigManager.hpp"

AgenticOrchestratorController::AgenticOrchestratorController() : m_running(false) {
    auto sandboxPath = AgenticConfigManager::getInstance().getSandboxPath();
    m_scriptDir = sandboxPath.string();
    m_binaryPath = (std::filesystem::current_path() / "build" / "AgenticPipeline").string();
}

AgenticOrchestratorController::~AgenticOrchestratorController() {
    stop();
}

void AgenticOrchestratorController::start(int port) {
    m_running = true;
    loadNodes();
    setupRoutes();
    m_maintenanceThread = std::thread(&AgenticOrchestratorController::maintenanceLoop, this);
    std::cout << "[ORCHESTRATOR] Agentic Supervisor listening on port " << port << std::endl;
    m_svr.listen("0.0.0.0", port);
}

void AgenticOrchestratorController::stop() {
    m_running = false;
    m_svr.stop();
    if (m_maintenanceThread.joinable()) m_maintenanceThread.join();
}

void AgenticOrchestratorController::loadNodes() {
    std::lock_guard<std::mutex> lock(m_nodesMutex);
    std::ifstream f((std::filesystem::path(m_scriptDir) / "nodes.json").string());
    if (f.is_open()) {
        f >> m_nodes;
    } else {
        m_nodes = nlohmann::json::array();
    }
}

void AgenticOrchestratorController::saveNodes() {
    std::lock_guard<std::mutex> lock(m_nodesMutex);
    std::ofstream f((std::filesystem::path(m_scriptDir) / "nodes.json").string());
    f << m_nodes.dump(2);
}

void AgenticOrchestratorController::setupRoutes() {
    m_svr.Get("/api/orchestrator/nodes", [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(m_nodesMutex);
        nlohmann::json enriched = nlohmann::json::array();
        for (auto& n : m_nodes) {
            auto node = n;
            httplib::Client cli(node["url"].get<std::string>());
            if (auto r = cli.Get("/api/state")) {
                if (r->status == 200) {
                    auto data = nlohmann::json::parse(r->body);
                    node["status"] = "ONLINE";
                    node["state"] = data.value("state", "UNKNOWN");
                } else {
                    node["status"] = "OFFLINE";
                }
            } else {
                node["status"] = "OFFLINE";
            }
            enriched.push_back(node);
        }
        res.set_content(enriched.dump(), "application/json");
    });

    m_svr.Post("/api/orchestrator/deploy", [this](const httplib::Request& req, httplib::Response& res) {
        auto data = nlohmann::json::parse(req.body);
        std::string name = data.value("name", "AgentNode_" + std::to_string(std::time(nullptr)));
        // Process deployment logic...
        res.set_content("{\"status\":\"agent_deployed\"}", "application/json");
    });
}

void AgenticOrchestratorController::maintenanceLoop() {
    while (m_running) {
        loadNodes();
        {
            std::lock_guard<std::mutex> lock(m_nodesMutex);
            for (auto& n : m_nodes) {
                // Node maintenance logic...
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}
