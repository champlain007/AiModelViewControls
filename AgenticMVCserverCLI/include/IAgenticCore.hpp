#pragma once
#include <string>
#include <memory>
#include <vector>

/**
 * @brief Exceptionally lightweight core interface for the Agentic Engine.
 * Follows the Facade and Strategy patterns.
 */
class IAgenticCore {
public:
    virtual ~IAgenticCore() = default;
    virtual bool initialize() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
    
    // Core Engine Functions (DockerEngine-like)
    virtual std::string executeTask(const std::string& taskPayload) = 0;
    virtual std::string getStatus() const = 0;
};

/**
 * @brief Security Interface for the CLI Engine.
 */
class IEngineSecurity {
public:
    virtual ~IEngineSecurity() = default;
    virtual bool scan(const std::string& data) = 0;
    virtual bool validateToken(const std::string& token) = 0;
};
