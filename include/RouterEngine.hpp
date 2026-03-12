#pragma once

#include "Model.hpp"
#include <thread>
#include <atomic>
#include <vector>

class RouterEngine {
public:
    RouterEngine(AgenticPipelineModel& model);
    ~RouterEngine();

    void start();
    void stop();

private:
    AgenticPipelineModel& m_model;
    std::thread m_loop;
    std::atomic<bool> m_running;

    void run(); 

    // Internal Handlers
    void handleFileTail(AgentHubConnector& conn);
    void handleFileWriter(AgentHubConnector& conn, const std::vector<AgentMessage>& pending);
    void handleUrlStream(AgentHubConnector& conn, const std::vector<AgentMessage>& pending);
    void handlePeerSync(AgentHubConnector& conn, const std::vector<AgentMessage>& backlog);
    void handleSystemIntf(AgentHubConnector& conn);
};
