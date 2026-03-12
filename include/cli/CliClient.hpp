#pragma once
#include <string>
#include <vector>

class AgenticCliClient {
public:
    AgenticCliClient(const std::string& host, int port);
    
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
};
