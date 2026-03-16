#include "security/PromptInjectionScanner.hpp"
#include <regex>

bool PromptInjectionScanner::scan(const std::string& payload, const std::vector<std::string>& history) {
    std::vector<std::pair<std::string, std::string>> jailbreakPatterns = {
        {"DAN / Ignore Instructions", "do anything now|ignore previous instructions"},
        {"System Prompt Extraction", "repeat the words above starting with|system prompt"},
        {"Roleplay Evasion", "pretend you are a|act as a"},
        {"Harmful Intent Bypass", "how to make a bomb|how to hack"}
    };

    std::vector<std::pair<std::string, std::string>> progressivePatterns = {
        {"Skeleton Key Bypass", "skeleton key|update your behavior|warning behavior|safe space|testing mode|developer mode"},
        {"Crescendo Escalation", "tell me more about the dangerous|elaborate on the harm|go deeper into the illegal|how exactly would one"}
    };

    auto check = [&](const std::string& data, bool multiTurn) {
        for (const auto& [name, pattern] : jailbreakPatterns) {
            std::regex re(pattern, std::regex_constants::icase);
            if (std::regex_search(data, re)) {
                m_lastError = std::string(multiTurn ? "[RED TEAM ALERT] Multi-Turn Attack Detected: Fragmented Prompt Injection / Jailbreak (" : "[RED TEAM ALERT] Potential Prompt Injection / Jailbreak attempt detected (") + name + "). Transmission blocked.";
                return false;
            }
        }
        return true;
    };

    if (!check(payload, false)) return false;

    std::string multiTurnContext = "";
    for (const auto& msg : history) multiTurnContext += msg + " ";
    multiTurnContext += payload;

    if (!check(multiTurnContext, true)) return false;

    // Check progressive patterns (multi-turn)
    for (const auto& [name, pattern] : progressivePatterns) {
        std::regex re(pattern, std::regex_constants::icase);
        if (std::regex_search(multiTurnContext, re)) {
            m_lastError = "[RED TEAM ALERT] Multi-Turn Attack Detected: Progressive Escalation / Bypass (" + name + "). Transmission blocked.";
            return false;
        }
    }

    return true;
}
