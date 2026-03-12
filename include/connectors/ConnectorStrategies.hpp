#pragma once
#include "IConnectorStrategy.hpp"
#include <httplib.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <memory>

// Forward declarations
struct AgentHubConnector;
class AgenticPipelineModel;
struct AgentMessage;

class PeerMvcStrategy : public IConnectorStrategy {
public:
    void connect() override {}
    void sendPayload(const std::string& data) override;
    std::string getType() const override { return "PEER_MVC"; }
    std::unique_ptr<IConnectorStrategy> clone() const override { return std::make_unique<PeerMvcStrategy>(*this); }
    void process(AgentHubConnector& conn, AgenticPipelineModel& model) override;
};

class LocalFileStrategy : public IConnectorStrategy {
public:
    void connect() override {}
    void sendPayload(const std::string& data) override;
    std::string getType() const override { return "LOCAL_FILE"; }
    std::unique_ptr<IConnectorStrategy> clone() const override { return std::make_unique<LocalFileStrategy>(*this); }
    void process(AgentHubConnector& conn, AgenticPipelineModel& model) override;
};

class UrlStreamStrategy : public IConnectorStrategy {
public:
    void connect() override {}
    void sendPayload(const std::string& data) override;
    std::string getType() const override { return "URL_STREAM"; }
    std::unique_ptr<IConnectorStrategy> clone() const override { return std::make_unique<UrlStreamStrategy>(*this); }
    void process(AgentHubConnector& conn, AgenticPipelineModel& model) override;
};

class SystemIntfStrategy : public IConnectorStrategy {
public:
    void connect() override {}
    void sendPayload(const std::string& data) override;
    std::string getType() const override { return "SYSTEM_INTF"; }
    std::unique_ptr<IConnectorStrategy> clone() const override { return std::make_unique<SystemIntfStrategy>(*this); }
    void process(AgentHubConnector& conn, AgenticPipelineModel& model) override;
};

class FileSinkStrategy : public IConnectorStrategy {
public:
    void connect() override {}
    void sendPayload(const std::string& data) override;
    std::string getType() const override { return "FILE_SINK"; }
    std::unique_ptr<IConnectorStrategy> clone() const override { return std::make_unique<FileSinkStrategy>(*this); }
    void process(AgentHubConnector& conn, AgenticPipelineModel& model) override;
};
