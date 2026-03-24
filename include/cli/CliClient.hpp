#pragma once
#include <string>
#include <vector>
#include "XWayland.hpp"

class AgenticCliClient {
public:
    AgenticCliClient(const std::string& host, int port);
    ~AgenticCliClient();
    
    int executeCommand(const std::vector<std::string>& args);

private:
    void printHelp();
    int cmdStatus();
    int cmdNodes();
    int cmdLedger();
    int cmdAlerts();
    int cmdSandbox(const std::vector<std::string>& args);
    
    std::string m_host;
    int m_port;
    XWayland m_wayland;
};
