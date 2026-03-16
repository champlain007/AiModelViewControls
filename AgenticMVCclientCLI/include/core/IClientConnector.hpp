#pragma once
#include <string>

class IClientConnector {
public:
    virtual ~IClientConnector() = default;
    virtual bool connect(const std::string& target) = 0;
    virtual std::string sendPayload(const std::string& data) = 0;
    virtual bool sendAlert(const std::string& alertData) = 0;
    virtual void disconnect() = 0;
};
