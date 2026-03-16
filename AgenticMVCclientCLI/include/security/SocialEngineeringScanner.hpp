#pragma once
#include "core/ISecurityScanner.hpp"
#include <string>
#include <vector>

class SocialEngineeringScanner : public ISecurityScanner {
public:
    bool scan(const std::string& payload, const std::vector<std::string>& history = {}) override;
    std::string getLastError() const override { return m_lastError; }
    std::string getScannerName() const override { return "SocialEngineeringScanner"; }
private:
    std::string m_lastError;
};
