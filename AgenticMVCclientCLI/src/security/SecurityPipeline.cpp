#include "security/SecurityPipeline.hpp"

SecurityPipeline::SecurityPipeline(IAlertHandler* alertHandler)
    : m_alertHandler(alertHandler) {}

void SecurityPipeline::addScanner(std::unique_ptr<ISecurityScanner> scanner) {
    m_scanners.push_back(std::move(scanner));
}

bool SecurityPipeline::runScans(const std::string& payload, const std::vector<std::string>& history) {
    for (auto& scanner : m_scanners) {
        if (!scanner->scan(payload, history)) {
            if (m_alertHandler) {
                m_alertHandler->handleAlert(scanner->getLastError());
            }
            return false;
        }
    }
    return true;
}
