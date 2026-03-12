#pragma once
#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

namespace security {

class AgentIdentityManager {
public:
    static bool validateToken(const std::string& token);
    static std::string generateToken(const std::string& agentId);
};

class AgentSandbox {
public:
    struct Policy {
        std::vector<std::string> allowedPaths;
        std::vector<std::string> allowedCommands;
        std::map<std::string, bool> overrides;
        bool allBlocked = false;
    };

    static nlohmann::json executeTool(const std::string& toolName, const nlohmann::json& arguments, const std::string& agentId, const std::string& token);
    static void updatePolicy(const std::string& tool, const std::string& action);

private:
    static Policy currentPolicy;
    static bool isPathAllowed(const std::string& path);
    static bool isCommandAllowed(const std::string& command);
};

struct AgentSecretRule {
    std::string id;
    std::string pattern;
    bool block;
};

std::vector<AgentSecretRule> getGlobalSecretPatterns();

} // namespace security
