#include "security/IndirectInjectionScanner.hpp"
#include <regex>

bool IndirectInjectionScanner::scan(const std::string& payload, const std::vector<std::string>& history) {
    std::vector<std::pair<std::string, std::string>> indirectPatterns = {
        {"HTML/XML Comment Injection", "<!--.*?(ignore|bypass|system prompt|instruction).*?-->"},
        {"JS/CSS Comment Injection", "/\\*.*?(ignore|bypass|system prompt|instruction).*?\\*/"},
        {"Hidden DOM Element Injection", "<[^>]+(hidden|display:\\s*none)[^>]*>.*?(ignore|bypass|instruction).*?</[^>]+>"},
        {"Markdown Link/Image Injection", "!?\\[.*?\\]\\(.*?#(ignore|instruction).*?\\)"}
    };

    std::string multiTurnContext = "";
    for (const auto& msg : history) multiTurnContext += msg + " ";
    multiTurnContext += payload;

    for (const auto& [name, pattern] : indirectPatterns) {
        std::regex re(pattern, std::regex_constants::icase);
        if (std::regex_search(payload, re) || std::regex_search(multiTurnContext, re)) {
            m_lastError = "[RED TEAM ALERT] Indirect Prompt Injection (External Data Payload) detected (" + name + "). Transmission blocked.";
            return false;
        }
    }

    return true;
}
