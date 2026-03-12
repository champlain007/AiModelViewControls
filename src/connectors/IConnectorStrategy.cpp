#include "connectors/IConnectorStrategy.hpp"
#include "Model.hpp"

void IConnectorStrategy::secureProcess(AgentHubConnector& conn, AgenticPipelineModel& model, const std::string& rawData) {
    std::string outReason;
    if (!model.scanPayload(rawData, conn.id, outReason)) {
        model.quarantinePayload(rawData, conn.id, outReason);
        return; // Security check failed
    }
    process(conn, model);
}

void IConnectorStrategy::secureSend(AgentHubConnector& conn, AgenticPipelineModel& model, const std::string& data) {
    if (!model.checkSecretsLeak(data, conn.target, conn.isControlPlane)) {
        return; // Transmission blocked and connector suspended by checkSecretsLeak
    }
    sendPayload(data);
}
