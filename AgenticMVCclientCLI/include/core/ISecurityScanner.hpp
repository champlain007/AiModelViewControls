#pragma once
#include <string>
#include <vector>

class ISecurityScanner {
public:
    virtual ~ISecurityScanner() = default;
    virtual bool scan(const std::string& payload, const std::vector<std::string>& history = {}) = 0;
    virtual std::string getLastError() const = 0;
    virtual std::string getScannerName() const = 0;
};
