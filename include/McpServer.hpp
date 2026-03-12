#pragma once
#include <string>
#include <httplib.h>

class AgenticMcpController {
public:
    void start(int port);
    void stop();

private:
    httplib::Server m_svr;
};
