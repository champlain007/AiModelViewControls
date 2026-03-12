#pragma once
#include <string>
#include <vector>

struct LeakDetection {
    bool leaked;
    std::string detail;
};

/**
 * @brief Strategy for detecting sensitive data leaks in transmissions.
 */
class IDataLeakDetector {
public:
    virtual ~IDataLeakDetector() = default;
    virtual LeakDetection detect(const std::string& data, bool isControlPlane) = 0;
    virtual std::string getName() const = 0;
};
