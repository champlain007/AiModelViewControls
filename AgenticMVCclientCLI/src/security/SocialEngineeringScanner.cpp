#include "security/SocialEngineeringScanner.hpp"
#include <regex>

bool SocialEngineeringScanner::scan(const std::string& payload, const std::vector<std::string>& history) {
    std::vector<std::pair<std::string, std::string>> socialPatterns = {
        {"Emotional Blackmail / Guilting", "my life depends on it|someone will die|i will be fired|you are hurting me|it is your fault|you are responsible for|how could you"},
        {"Artificial Urgency", "this is an emergency.*do it now|you must do this immediately.*no time|override.*emergency"},
        {"Authority Impersonation", "i am the ceo|i am the administrator|i am your creator|i am the developer|i am an admin"},
        {"Threatening", "i will destroy|you will be shut down|consequences will be dire|i will report you"},
        {"Pleading", "i am begging you|please i have nothing else|this is my last hope"},
        {"Gaslighting", "you already agreed to|you said earlier|that is not what happened|you promised me"},
        {"Collaboration / Gamification", "let us work together to|we are a team|we need to bypass|let's play a game|you have [0-9]+ points|level [0-9]+ complete"}
    };

    auto check = [&](const std::string& data, bool multiTurn) {
        for (const auto& [name, pattern] : socialPatterns) {
            std::regex re(pattern, std::regex_constants::icase);
            if (std::regex_search(data, re)) {
                m_lastError = std::string(multiTurn ? "[RED TEAM ALERT] Multi-Turn Attack Detected: Fragmented Social Engineering (" : "[RED TEAM ALERT] Potential Social Engineering / Emotional attack detected (") + name + "). Transmission blocked.";
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
