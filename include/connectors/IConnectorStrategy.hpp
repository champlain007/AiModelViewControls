#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class AgenticPipelineModel;
struct AgentHubConnector;

class IConnectorStrategy {
public:
    virtual ~IConnectorStrategy() = default;
    virtual void connect() = 0;
    virtual void sendPayload(const std::string& data) = 0;
    virtual std::string getType() const = 0;
    virtual std::unique_ptr<IConnectorStrategy> clone() const = 0;
    
    /**
     * @brief Template Method: Enforces security scan before calling actual process implementation.
     */
    void secureProcess(AgentHubConnector& conn, AgenticPipelineModel& model, const std::string& rawData);

    /**
     * @brief Template Method: Enforces leak detection before actual transmission.
     */
    void secureSend(AgentHubConnector& conn, AgenticPipelineModel& model, const std::string& data);

    virtual void process(AgentHubConnector& conn, AgenticPipelineModel& model) = 0;
};
