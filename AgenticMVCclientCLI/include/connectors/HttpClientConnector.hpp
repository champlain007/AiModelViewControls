#pragma once
#include "core/IClientConnector.hpp"
#include <string>

class HttpClientConnector : public IClientConnector {
public:
    bool connect(const std::string& target) override;
    std::string sendPayload(const std::string& data) override;
    bool sendAlert(const std::string& alertData) override;
    void disconnect() override;
private:
    std::string m_target;
};
