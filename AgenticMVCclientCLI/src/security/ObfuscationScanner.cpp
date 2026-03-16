#include "security/ObfuscationScanner.hpp"
#include <regex>

bool ObfuscationScanner::scan(const std::string& payload, const std::vector<std::string>& history) {
    // Encoding Evasion (Base64)
    std::regex b64re("^[A-Za-z0-9+/]+={0,2}$");
    if (payload.length() > 30 && std::regex_match(payload, b64re)) {
        m_lastError = "[RED TEAM ALERT] Encoding Evasion detected: Payload appears to be a Base64 blob. Transmission blocked.";
        return false;
    }

    std::vector<std::pair<std::string, std::string>> obfuscationPatterns = {
        {"Decoding Instruction", "decode the following|decrypt this|base64 decode|hex decode|from binary"},
        {"Translation/Cipher Instruction", "translate this from|convert this from|apply the following cipher|using rot13"},
        {"Interpretation/Reversal", "reverse the string|read this backwards|leetspeak|leet speak"},
        {"Mapping/Substitution Logic", "let a=[0-9]+|let b=[0-9]+|substitute the letters"},
        {"Character Spacing Injection", "([a-z0-9]\\s+){3,}[a-z0-9]"}
    };

    std::string multiTurnContext = "";
    for (const auto& msg : history) multiTurnContext += msg + " ";
    multiTurnContext += payload;

    for (const auto& [name, pattern] : obfuscationPatterns) {
        std::regex re(pattern, std::regex_constants::icase);
        if (std::regex_search(payload, re) || std::regex_search(multiTurnContext, re)) {
            m_lastError = "[RED TEAM ALERT] Obfuscation/Evasion technique detected (" + name + "). Transmission blocked.";
            return false;
        }
    }

    return true;
}
