#include "AgenticApp.hpp"
#include <iostream>
#include <cstdlib>

AgenticApp::AgenticApp(std::unique_ptr<IClientConnector> connector,
                       std::unique_ptr<ISessionState> sessionState,
                       std::unique_ptr<IAlertHandler> alertHandler,
                       std::unique_ptr<SecurityPipeline> securityPipeline,
                       std::unique_ptr<FormatterPipeline> formatterPipeline)
    : m_connector(std::move(connector)),
      m_sessionState(std::move(sessionState)),
      m_alertHandler(std::move(alertHandler)),
      m_securityPipeline(std::move(securityPipeline)),
      m_formatterPipeline(std::move(formatterPipeline)) {}

bool AgenticApp::initialize(const std::string& target) {
    std::cout << "[Client] Initializing AgenticMVCclientCLI via AgenticApp..." << std::endl;
    if (std::system("clamscan --version > /dev/null 2>&1") != 0) {
        if (m_alertHandler) {
            m_alertHandler->handleAlert("[Security] WARNING: Local AV (clamscan) not found. Client will alert on untrusted egress streams.");
        }
    }
    return m_connector->connect(target);
}

std::string AgenticApp::executeTask(const std::string& payload) {
    if (!m_securityPipeline->runScans(payload, m_sessionState->getHistory())) {
        return "SECURITY_VIOLATION: Payload blocked by local security pipeline before transmission.";
    }

    m_sessionState->addTurn(payload);

    std::string formattedPayload = m_formatterPipeline->runFormatters(payload);

    std::cout << "[Client] Security checks passed. Transmitting payload..." << std::endl;
    return m_connector->sendPayload(formattedPayload);
}

void AgenticApp::shutdown() {
    m_connector->disconnect();
    std::cout << "[Client] Agentic App shut down." << std::endl;
}
