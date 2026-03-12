#include "Security.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>
#include <array>
#include <memory>
#include <cstdio>
#include <ctime>

namespace security {

// OS Generic default sandbox paths
AgentSandbox::Policy AgentSandbox::currentPolicy = {
    {(std::filesystem::temp_directory_path() / "mcp_safe").string(), "www/"},
    {"ls", "echo", "date"}
};

bool AgentIdentityManager::validateToken(const std::string& token) {
    if (token.length() < 16) return false;
    for (char c : token) {
        if (!isalnum(c)) return false;
    }
    return true;
}

std::string AgentIdentityManager::generateToken(const std::string& agentId) {
    // Simplified token generation for Agentic prototype
    return "AGENT_TOKEN_" + agentId + "_" + std::to_string(std::time(nullptr));
}

bool AgentSandbox::isPathAllowed(const std::string& path) {
    std::string realPath = std::filesystem::weakly_canonical(path).string();
    for (const auto& allowed : currentPolicy.allowedPaths) {
        std::string realAllowed = std::filesystem::weakly_canonical(allowed).string();
        if (realPath.compare(0, realAllowed.length(), realAllowed) == 0) return true;
    }
    return false;
}

bool AgentSandbox::isCommandAllowed(const std::string& command) {
    std::string baseCmd = command.substr(0, command.find(' '));
    for (const auto& allowed : currentPolicy.allowedCommands) {
        if (baseCmd == allowed) return true;
    }
    return false;
}

void AgentSandbox::updatePolicy(const std::string& tool, const std::string& action) {
    if (tool == "ALL") {
        currentPolicy.allBlocked = (action == "BLOCK");
    } else {
        currentPolicy.overrides[tool] = (action == "BLOCK");
    }
}

nlohmann::json AgentSandbox::executeTool(const std::string& toolName, const nlohmann::json& arguments, const std::string& agentId, const std::string& token) {
    if (!AgentIdentityManager::validateToken(token)) {
        return {{"status", "UNAUTHORIZED"}, {"message", "Invalid or missing agent token."}};
    }

    if (currentPolicy.allBlocked || (currentPolicy.overrides.count(toolName) && currentPolicy.overrides[toolName])) {
        return {{"status", "OVERRIDDEN"}, {"message", "Action blocked by Global Agentic Orchestrator."}};
    }

    if (toolName == "read_file") {
        std::string path = arguments.value("path", "");
        if (!isPathAllowed(path)) {
            return {{"status", "SANDBOX_VIOLATION"}, {"message", "Path is outside agent sandbox boundaries."}};
        }
        try {
            std::ifstream f(path);
            std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            if (content.length() > 1024) content = content.substr(0, 1024);
            return {{"status", "SUCCESS"}, {"content", content}};
        } catch (const std::exception& e) {
            return {{"status", "ERROR"}, {"message", e.what()}};
        }
    } else if (toolName == "command_execution") {
        std::string cmd = arguments.value("command", "");
        if (!isCommandAllowed(cmd)) {
            return {{"status", "SANDBOX_VIOLATION"}, {"message", "Command is not in the allowed agent sandbox list."}};
        }
#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&PCLOSE)> pipe(POPEN(cmd.c_str(), "r"), PCLOSE);
        if (!pipe) return {{"status", "ERROR"}, {"message", "Failed to start agent command."}};
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return {{"status", "SUCCESS"}, {"output", result}};
    }

    return {{"status", "UNKNOWN_TOOL"}};
}

std::vector<AgentSecretRule> getGlobalSecretPatterns() {
    return {
        {"Block_OpenAI_Key", "sk-[a-zA-Z0-9]{20,}", true},
        {"Block_Anthropic_Key", "sk-ant-[a-zA-Z0-9-]{20,}", true},
        {"Block_GitHub_Token", "ghp_[a-zA-Z0-9]{36}", true},
        {"Block_PrivateKey", "-----BEGIN.*PRIVATE KEY", true}
    };
}

} // namespace security
