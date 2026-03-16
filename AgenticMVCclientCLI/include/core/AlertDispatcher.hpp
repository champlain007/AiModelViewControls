#pragma once
#include "IAlertHandler.hpp"
#include "IClientConnector.hpp"
#include <vector>
#include <string>
#include <memory>

class AlertDispatcher : public IAlertHandler {
public:
    AlertDispatcher(IClientConnector* connector, bool syncAlerts);
    void handleAlert(const std::string& message) override;
private:
    IClientConnector* m_connector;
    bool m_syncAlerts;
};
