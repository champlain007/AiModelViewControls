#pragma once
#include "core/IClientConnector.hpp"
#include "core/ISessionState.hpp"
#include "core/IAlertHandler.hpp"
#include "security/SecurityPipeline.hpp"
#include "formatters/FormatterPipeline.hpp"
#include <memory>
#include <string>

class AgenticApp {
public:
    AgenticApp(std::unique_ptr<IClientConnector> connector,
               std::unique_ptr<ISessionState> sessionState,
               std::unique_ptr<IAlertHandler> alertHandler,
               std::unique_ptr<SecurityPipeline> securityPipeline,
               std::unique_ptr<FormatterPipeline> formatterPipeline);
    
    bool initialize(const std::string& target);
    std::string executeTask(const std::string& payload);
    void shutdown();

private:
    std::unique_ptr<IClientConnector> m_connector;
    std::unique_ptr<ISessionState> m_sessionState;
    std::unique_ptr<IAlertHandler> m_alertHandler;
    std::unique_ptr<SecurityPipeline> m_securityPipeline;
    std::unique_ptr<FormatterPipeline> m_formatterPipeline;
};
