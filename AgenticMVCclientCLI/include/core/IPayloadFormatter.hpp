#pragma once
#include <string>

class IPayloadFormatter {
public:
    virtual ~IPayloadFormatter() = default;
    virtual std::string format(const std::string& payload) = 0;
};
