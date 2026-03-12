#pragma once
#include "Model.hpp"
#include <thread>
#include <atomic>

class AgenticEngine {
public:
    AgenticEngine(AgenticPipelineModel& model);
    ~AgenticEngine();

    void start();
    void stop();

private:
    AgenticPipelineModel& m_model;
    std::thread m_loop;
    std::atomic<bool> m_running;

    void run(); // Autonomous thinking loop
};
