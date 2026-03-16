#pragma once
#include <string>

class IAlertHandler {
public:
    virtual ~IAlertHandler() = default;
    virtual void handleAlert(const std::string& message) = 0;
};
