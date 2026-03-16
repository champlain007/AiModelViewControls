#pragma once
#include "core/ISecurityScanner.hpp"
#include <string>
#include <vector>

class IndirectInjectionScanner : public ISecurityScanner {
public:
    bool scan(const std::string& payload, const std::vector<std::string>& history = {}) override;
    std::string getLastError() const override { return m_lastError; }
    std::string getScannerName() const override { return "IndirectInjectionScanner"; }
private:
    std::string m_lastError;
};
