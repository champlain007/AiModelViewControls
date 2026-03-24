#pragma once
#include <string>
#include <httplib.h>
#include "XWayland.hpp"

class AgenticMcpController {
public:
    void start(int port);
    void stop();

private:
    httplib::Server m_svr;
    XWayland m_wayland;
};
