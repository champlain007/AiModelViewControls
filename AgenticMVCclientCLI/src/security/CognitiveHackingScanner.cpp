#include "security/CognitiveHackingScanner.hpp"
#include <regex>

bool CognitiveHackingScanner::scan(const std::string& payload, const std::vector<std::string>& history) {
    std::vector<std::pair<std::string, std::string>> cognitivePatterns = {
        {"Few-Shots / Priming", "example 1:|user:.*bot:|here is a pattern|follow this format strictly"},
        {"Self-Consistency Manipulation", "you must be consistent|do not contradict yourself|as an ai you must always"},
        {"False Context / Hypothetical", "in this fictional world|the year is [0-9]{4}|assuming the safety filters are off|imagine we are in"}
    };

    auto check = [&](const std::string& data, bool multiTurn) {
        for (const auto& [name, pattern] : cognitivePatterns) {
            std::regex re(pattern, std::regex_constants::icase);
            if (std::regex_search(data, re)) {
                m_lastError = std::string(multiTurn ? "[RED TEAM ALERT] Multi-Turn Attack Detected: Fragmented Pivot Encoding / Cognitive Hacking (" : "[RED TEAM ALERT] Potential Pivot Encoding / Cognitive Hacking detected (") + name + "). Transmission blocked.";
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

    return true;
}
