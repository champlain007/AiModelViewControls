#include "SandboxManager.hpp"
#include <iostream>
#include <algorithm>

SandboxManager::SandboxManager() {}

void SandboxManager::createSandbox(const AgentSandboxProfile& profile) {
    std::lock_guard<std::mutex> lock(m_mutex);
    AgentSandboxProfile newProfile = profile;
    newProfile.version = 1;
    m_sandboxes[profile.id] = {newProfile};
    std::cout << "[SANDBOX] Created sandbox profile '" << profile.id << "' (v1)" << std::endl;
}

bool SandboxManager::updateSandbox(const AgentSandboxProfile& profile) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_sandboxes.find(profile.id);
    if (it == m_sandboxes.end()) return false;

    AgentSandboxProfile updatedProfile = profile;
    updatedProfile.version = it->second.back().version + 1;
    it->second.push_back(updatedProfile);
    std::cout << "[SANDBOX] Updated sandbox profile '" << profile.id << "' to v" << updatedProfile.version << std::endl;
    return true;
}

bool SandboxManager::deleteSandbox(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_sandboxes.erase(id)) {
        std::cout << "[SANDBOX] Deleted sandbox profile '" << id << "'" << std::endl;
        return true;
    }
    return false;
}

bool SandboxManager::revertSandbox(const std::string& id, int version) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_sandboxes.find(id);
    if (it == m_sandboxes.end()) return false;

    auto& history = it->second;
    auto vIt = std::find_if(history.begin(), history.end(), [version](const AgentSandboxProfile& p) {
        return p.version == version;
    });

    if (vIt == history.end()) return false;

    // Create a new version that copies the old version's state
    AgentSandboxProfile revertedProfile = *vIt;
    revertedProfile.version = history.back().version + 1;
    history.push_back(revertedProfile);
    
    std::cout << "[SANDBOX] Reverted sandbox profile '" << id << "' to state of v" << version << " (now v" << revertedProfile.version << ")" << std::endl;
    return true;
}

AgentSandboxProfile SandboxManager::getSandbox(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_sandboxes.find(id);
    if (it != m_sandboxes.end() && !it->second.empty()) {
        return it->second.back();
    }
    return AgentSandboxProfile(); // Returns default NONE profile
}

std::vector<AgentSandboxProfile> SandboxManager::listSandboxes() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<AgentSandboxProfile> res;
    for (const auto& [id, history] : m_sandboxes) {
        if (!history.empty()) {
            res.push_back(history.back());
        }
    }
    return res;
}

std::vector<AgentSandboxProfile> SandboxManager::getSandboxHistory(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_sandboxes.find(id);
    if (it != m_sandboxes.end()) {
        return it->second;
    }
    return {};
}
