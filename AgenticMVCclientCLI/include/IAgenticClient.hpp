#pragma once
#include <string>

/**
 * @brief Strategy Interface for Client Connection Methods.
 * Provides a polymorphic way to send data (e.g., HTTP, WebSockets, or Local IPC)
 */
class IClientConnector {
public:
    virtual ~IClientConnector() = default;
    virtual bool connect(const std::string& target) = 0;
    virtual std::string sendPayload(const std::string& data) = 0;
    virtual void disconnect() = 0;
};
