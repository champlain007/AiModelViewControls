#pragma once
#include "core/ISecurityScanner.hpp"
#include "core/IAlertHandler.hpp"
#include <vector>
#include <memory>

class SecurityPipeline {
public:
    SecurityPipeline(IAlertHandler* alertHandler);
    void addScanner(std::unique_ptr<ISecurityScanner> scanner);
    bool runScans(const std::string& payload, const std::vector<std::string>& history);
private:
    IAlertHandler* m_alertHandler;
    std::vector<std::unique_ptr<ISecurityScanner>> m_scanners;
};
