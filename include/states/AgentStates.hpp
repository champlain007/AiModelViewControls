#pragma once
#include "IAgentState.hpp"

class NormalState : public IAgentState {
public:
    bool handleRequest() const override { return true; }
    bool canConnect() const override { return true; }
    std::string getName() const override { return "NORMAL"; }
};

class WarningState : public IAgentState {
public:
    bool handleRequest() const override { return true; }
    bool canConnect() const override { return true; }
    std::string getName() const override { return "WARNING"; }
};

class LockdownState : public IAgentState {
public:
    bool handleRequest() const override { return false; }
    bool canConnect() const override { return false; }
    std::string getName() const override { return "LOCKDOWN"; }
};
