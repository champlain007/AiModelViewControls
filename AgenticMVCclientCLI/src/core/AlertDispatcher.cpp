#include "core/AlertDispatcher.hpp"
#include <iostream>

AlertDispatcher::AlertDispatcher(IClientConnector* connector, bool syncAlerts)
    : m_connector(connector), m_syncAlerts(syncAlerts) {}

void AlertDispatcher::handleAlert(const std::string& message) {
    std::cerr << message << std::endl;
    if (m_syncAlerts && m_connector) {
        std::cout << "[Client] Syncing alert to Orchestrator..." << std::endl;
        m_connector->sendAlert(message);
    }
}
