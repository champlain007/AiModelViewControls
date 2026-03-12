#pragma once
#include <string>

class IAgentState {
public:
    virtual ~IAgentState() = default;
    virtual bool handleRequest() const = 0;
    virtual bool canConnect() const = 0;
    virtual std::string getName() const = 0;
};
