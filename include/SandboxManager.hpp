#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class AgentSandboxLevel { NONE, PERMISSIVE, RESTRICTED, PARANOID };

struct AgentSandboxProfile {
    std::string id;
    int version = 1;
    AgentSandboxLevel level = AgentSandboxLevel::NONE;
    std::vector<std::string> allowedPaths;   // For LOCAL_FILE / FILE_SINK
    std::vector<std::string> allowedDomains; // For URL_STREAM / PEER_MVC
    bool allowNetwork = true;
    bool allowDiskWrite = false;
    
    json toJson() const {
        return json{
            {"id", id},
            {"version", version},
            {"level", (int)level},
            {"allowedPaths", allowedPaths},
            {"allowedDomains", allowedDomains},
            {"allowNetwork", allowNetwork},
            {"allowDiskWrite", allowDiskWrite}
        };
    }

    static AgentSandboxProfile fromJson(const json& j) {
        AgentSandboxProfile p;
        p.id = j.value("id", "");
        p.version = j.value("version", 1);
        p.level = (AgentSandboxLevel)j.value("level", 0);
        if (j.contains("allowedPaths")) p.allowedPaths = j["allowedPaths"].get<std::vector<std::string>>();
        if (j.contains("allowedDomains")) p.allowedDomains = j["allowedDomains"].get<std::vector<std::string>>();
        p.allowNetwork = j.value("allowNetwork", true);
        p.allowDiskWrite = j.value("allowDiskWrite", false);
        return p;
    }
};

class SandboxManager {
public:
    SandboxManager();
    
    void createSandbox(const AgentSandboxProfile& profile);
    bool updateSandbox(const AgentSandboxProfile& profile); // Creates a new version
    bool deleteSandbox(const std::string& id);
    bool revertSandbox(const std::string& id, int version);
    
    AgentSandboxProfile getSandbox(const std::string& id) const;
    std::vector<AgentSandboxProfile> listSandboxes() const;
    std::vector<AgentSandboxProfile> getSandboxHistory(const std::string& id) const;

private:
    mutable std::mutex m_mutex;
    // Map of sandbox ID -> History of profiles (index 0 is v1, index 1 is v2, etc.)
    std::map<std::string, std::vector<AgentSandboxProfile>> m_sandboxes;
};
