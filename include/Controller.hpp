#pragma once

#include "Model.hpp"
#include <httplib.h>
#include <thread>
#include <atomic>
#include "XWayland.hpp"

class AgenticHttpController {
public:
    AgenticHttpController(AgenticPipelineModel& model, int port = 8080);
    ~AgenticHttpController();

    void start();
    void stop();

private:
    AgenticPipelineModel& m_model;
    httplib::Server m_svr;
    int m_port;

    std::thread m_serverThread;
    std::thread m_pollerThread; // Background thread for polling
    std::atomic<bool> m_running;
    XWayland m_wayland;

    void setupRoutes();
    void runPoller(); // Polling loop
    void executePoll(const AgentResource& res);
};
