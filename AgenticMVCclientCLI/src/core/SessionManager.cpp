#include "core/SessionManager.hpp"

void SessionManager::addTurn(const std::string& payload) {
    m_history.push_back(payload);
    if (m_history.size() > 5) {
        m_history.erase(m_history.begin());
    }
}

std::vector<std::string> SessionManager::getHistory() const {
    return m_history;
}

void SessionManager::clear() {
    m_history.clear();
}
