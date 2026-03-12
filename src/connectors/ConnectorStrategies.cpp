#include "connectors/ConnectorStrategies.hpp"
#include "Model.hpp"
#include <httplib.h>
#include <fstream>
#include <iostream>

static std::map<std::string, std::streampos> m_fileOffsets;

void PeerMvcStrategy::sendPayload(const std::string& data) {
    // PeerMvc typically sends in batches in the current RouterEngine
}

void PeerMvcStrategy::process(AgentHubConnector& conn, AgenticPipelineModel& model) {
    // (Actual logic would be moved from RouterEngine here)
}

void LocalFileStrategy::sendPayload(const std::string& data) {
    // LocalFile is typically a source, not a sink
}

void LocalFileStrategy::process(AgentHubConnector& conn, AgenticPipelineModel& model) {
    if (!model.validateSandboxAction(conn, "DISK_READ", conn.target)) return;

    std::ifstream file(conn.target);
    if (!file.is_open()) return;

    if (m_fileOffsets.find(conn.id) == m_fileOffsets.end()) {
        file.seekg(0, std::ios::end);
        m_fileOffsets[conn.id] = file.tellg();
    }

    file.seekg(m_fileOffsets[conn.id]);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        secureProcess(conn, model, line);
    }
    m_fileOffsets[conn.id] = file.tellg();
}

void UrlStreamStrategy::sendPayload(const std::string& data) {
    // Implementation for URL stream sink
}

void UrlStreamStrategy::process(AgentHubConnector& conn, AgenticPipelineModel& model) {
    // Logic for processing URL streams as sources
}

void SystemIntfStrategy::sendPayload(const std::string& data) {}

void SystemIntfStrategy::process(AgentHubConnector& conn, AgenticPipelineModel& model) {
    if (conn.metadata.contains("mac")) {
        AgentMessage msg;
        msg.senderId = "AgenticSysIntf";
        msg.topic = "intf_monitor";
        msg.content = "Agent Interface " + conn.target + " (MAC: " + conn.metadata["mac"].get<std::string>() + ") active.";
        msg.timestamp = std::chrono::system_clock::now();
        model.pushMessage(msg);
    }
}

void FileSinkStrategy::sendPayload(const std::string& data) {
    // Actual implementation for writing to file
}

void FileSinkStrategy::process(AgentHubConnector& conn, AgenticPipelineModel& model) {
    // Logic for processing file sink as source if needed
}
