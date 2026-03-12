#include "AgenticEngine.hpp"
#include "states/AgentStates.hpp"
#include <iostream>
#include <memory>

AgenticEngine::AgenticEngine(AgenticPipelineModel& model) : m_model(model), m_running(false) {}

AgenticEngine::~AgenticEngine() { stop(); }

void AgenticEngine::start() {
    m_running = true;
    m_loop = std::thread(&AgenticEngine::run, this);
}

void AgenticEngine::stop() {
    if (m_running) {
        m_running = false;
        if (m_loop.joinable()) m_loop.join();
    }
}

void AgenticEngine::run() {
    std::cout << "[AGENT_SYSTEM] MainHub: Autonomous AI Thinking (Agentic Logic) initialized." << std::endl;
    while (m_running) {
        auto alerts = m_model.getRecentAlerts(10);
        
        // 1. Autonomous Agent Reasoning & Response
        if (alerts.size() >= 5) {
            if (m_model.getState()->getName() != "LOCKDOWN") {
                m_model.setState(std::make_unique<LockdownState>());
                m_model.triggerAlert("AUTONOMOUS_LOCKDOWN", "AgenticBrain", "Critical alert volume detected. Activating full agentic isolation.");
                m_model.getCrypto().rotateKey();
            }
        } else if (alerts.size() >= 2) {
            if (m_model.getState()->getName() == "NORMAL") {
                m_model.setState(std::make_unique<WarningState>());
                m_model.triggerAlert("ELEVATED_RISK", "AgenticBrain", "Anomaly threshold exceeded. Increasing agent surveillance.");
            }
        } else {
            if (m_model.getState()->getName() != "NORMAL") {
                m_model.setState(std::make_unique<NormalState>());
            }
        }

        // 2. Continuous Agent Policy Enforcement
        static bool init = false;
        if (!init) {
            AgentScannerConfig sc;
            sc.enabled = true;
            m_model.configureScanner(sc);

            m_model.addFirewallRule({"AGENT-WAF-01", "SELECT.*FROM", true});
            m_model.addFirewallRule({"AGENT-WAF-02", "<script>", true});
            m_model.addFirewallRule({"AGENT-WAF-03", "../", true}); 
            init = true;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}
