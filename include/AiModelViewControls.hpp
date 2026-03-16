#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "Orchestrator.hpp"

/**
 * @brief Sub-class/Component responsible for managing Agentic Node pipelines and connections.
 */
class AgenticMVCpipe {
public:
    AgenticMVCpipe(int orchPort) : m_orchPort(orchPort) {}
    
    void createPipeline(const std::vector<std::string>& nodeUrls) {
        std::cout << "[Pipeline] Creating connection chain for " << nodeUrls.size() << " nodes..." << std::endl;
        for (const auto& url : nodeUrls) {
            std::cout << "[Pipeline] Connecting to node: " << url << std::endl;
        }
    }

    int getPort() const { return m_orchPort; }

private:
    int m_orchPort;
};

/**
 * @brief Main Facade Class for the AiModelViewControls framework.
 */
class AiModelViewControls {
public:
    AiModelViewControls() : m_pipeline(std::make_unique<AgenticMVCpipe>(9000)) {}

    void initialize(int port) {
        std::cout << "[AiMVCs] Initializing AiModelViewControls on port " << port << "..." << std::endl;
        m_orchestrator = std::make_unique<AgenticOrchestratorController>();
        m_pipeline = std::make_unique<AgenticMVCpipe>(port);
    }

    void start(int port) {
        if (m_orchestrator) {
            std::cout << "[AiMVCs] Launching Orchestrator..." << std::endl;
            m_orchestrator->start(port);
        }
    }

    AgenticMVCpipe* getPipeline() { return m_pipeline.get(); }

private:
    std::unique_ptr<AgenticOrchestratorController> m_orchestrator;
    std::unique_ptr<AgenticMVCpipe> m_pipeline;
};
