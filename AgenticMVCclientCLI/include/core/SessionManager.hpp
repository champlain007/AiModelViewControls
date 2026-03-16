#pragma once
#include "ISessionState.hpp"
#include <vector>
#include <string>

class SessionManager : public ISessionState {
public:
    void addTurn(const std::string& payload) override;
    std::vector<std::string> getHistory() const override;
    void clear() override;
private:
    std::vector<std::string> m_history;
};
