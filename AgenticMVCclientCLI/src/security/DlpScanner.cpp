#include "security/DlpScanner.hpp"
#include <regex>

bool DlpScanner::scan(const std::string& payload, const std::vector<std::string>& history) {
    std::vector<std::pair<std::string, std::string>> dlpPatterns = {
        {"RSA PRIVATE KEY", "BEGIN RSA PRIVATE KEY"},
        {"Generic Secret Key", "TRADESECRET_HANDSHAKE_KEY"},
        {"AWS Access Key", "AKIA[0-9A-Z]{16}"},
        {"Email Address", "[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}"}
    };

    for (const auto& [name, pattern] : dlpPatterns) {
        std::regex re(pattern);
        if (std::regex_search(payload, re)) {
            m_lastError = "[ALERT] DLP: Potential " + name + " leak detected. Transmission blocked.";
            return false;
        }
    }
    return true;
}
