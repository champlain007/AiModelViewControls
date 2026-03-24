#pragma once
#include <string>
#include <vector>
#include "XWayland.hpp"

// Forward-declare httplib::Client to avoid including the full header here
namespace httplib { class Client; }

class DockerFacade {
public:
    DockerFacade(const std::string& host, int port);
    ~DockerFacade();

    int execute(const std::vector<std::string>& args);

private:
    void printHelp();
    int cmdPs();
    int cmdRun(const std::vector<std::string>& args);
    int cmdStop(const std::vector<std::string>& args);
    int cmdInspect(const std::vector<std::string>& args);

    std::string m_host;
    int m_port;
    httplib::Client* m_cli;
    XWayland m_wayland;
};
