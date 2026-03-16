#pragma once
#include "AgenticApp.hpp"

class CliInteractor {
public:
    CliInteractor(AgenticApp& app, bool useHttp);
    void run();
private:
    AgenticApp& m_app;
    bool m_useHttp;
};
