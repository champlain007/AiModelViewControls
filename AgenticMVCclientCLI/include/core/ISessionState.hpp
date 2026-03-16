#pragma once
#include <string>
#include <vector>

class ISessionState {
public:
    virtual ~ISessionState() = default;
    virtual void addTurn(const std::string& payload) = 0;
    virtual std::vector<std::string> getHistory() const = 0;
    virtual void clear() = 0;
};
