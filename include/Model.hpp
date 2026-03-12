#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <chrono>
#include <regex>
#include <memory>
#include <nlohmann/json.hpp>

#include "AgenticConfigManager.hpp"
#include "Crypto.hpp"
#include "states/IAgentState.hpp"
#include "connectors/IConnectorStrategy.hpp"
#include "SandboxManager.hpp"
#include "Ledger.hpp"
#include "security/IMalwareScanner.hpp"
#include "security/ConcreteScanners.hpp"
#include "security/IDataLeakDetector.hpp"

using json = nlohmann::json;

// --- AGENT TOKENIZATION SYSTEM ---
enum class AgentTokenScope { INTERNAL, EXTERNAL, GLOBAL_SYSTEM };

struct AgentQuarantineEntry {
    std::string id;
    std::string contentPreview;
    std::string encryptedContent;
    std::string origin;
    std::string reason;
    std::chrono::system_clock::time_point timestamp;
    bool resolved = false;
    json toJson() const;
};

struct AgentAuditEntry {
    std::string id;
    std::string action;
    std::string component;
    std::string details;
    std::string status;
    std::chrono::system_clock::time_point timestamp;
    json toJson() const;
};

struct AgentScannerConfig {
    bool enabled = true;
    std::string defaultCmd = "clamscan --no-summary";
    std::string customCmd;
    bool blockOnError = true;
};

struct AgentMessage {
    std::string senderId;
    std::string topic;
    std::string content;
    bool encrypted = false;
    bool isLocal = true;
    std::chrono::system_clock::time_point timestamp;
    json toJson() const;
};

struct AgentConnectionSecret {
    std::string id;
    std::string type;
    std::string keyData;
    std::string groupName;
    bool isControlPlane = false;
    json toJson() const {
        return json{{"id", id}, {"type", type}, {"keyData", keyData}, {"groupName", groupName}, {"isControlPlane", isControlPlane}};
    }
};

struct AgentHubConnector {
    std::string id;
    std::unique_ptr<IConnectorStrategy> strategy;
    std::string target;
    json metadata;
    bool active = true;
    bool suspended = false;
    bool isControlPlane = false;
    AgentSandboxProfile sandbox;

    AgentHubConnector() = default;
    AgentHubConnector(const AgentHubConnector& other) : 
        id(other.id), target(other.target), metadata(other.metadata), 
        active(other.active), suspended(other.suspended), 
        isControlPlane(other.isControlPlane), sandbox(other.sandbox) {
        if (other.strategy) strategy = other.strategy->clone();
    }
    AgentHubConnector& operator=(const AgentHubConnector& other) {
        if (this != &other) {
            id = other.id;
            target = other.target;
            metadata = other.metadata;
            active = other.active;
            suspended = other.suspended;
            isControlPlane = other.isControlPlane;
            sandbox = other.sandbox;
            if (other.strategy) strategy = other.strategy->clone();
            else strategy.reset();
        }
        return *this;
    }
    AgentHubConnector(AgentHubConnector&&) = default;
    AgentHubConnector& operator=(AgentHubConnector&&) = default;
};

struct AgentResource {
    std::string id;
    std::string type;
    std::string target;
    int intervalMs;
    std::chrono::system_clock::time_point lastPoll;
};

struct AgentFirewallRule {
    std::string id;
    std::string pattern;
    bool blockRequest;
};

struct AgentAlert {
    std::chrono::system_clock::time_point timestamp;
    std::string type;
    std::string agentId;
    std::string details;
    json toJson() const;
};

struct AgentSystemToken {
    std::string token;
    std::string ownerId;
    AgentTokenScope scope;
    std::chrono::system_clock::time_point expiry;
    bool active = true;
};

class AgenticPipelineModel {
public:
    AgenticPipelineModel();
    
    // Core
    void pushMessage(const AgentMessage& msg);
    std::vector<AgentMessage> getRecentMessages(int limit = 50);

    // Distributed Hub
    void addConnector(const AgentHubConnector& conn);
    void removeConnector(const std::string& id);
    std::vector<AgentHubConnector> getConnectors();
    
    // Token Engine
    std::string issueToken(const std::string& ownerId, AgentTokenScope scope, int ttlMinutes = 60);
    bool validateToken(const std::string& token, AgentTokenScope requiredScope);
    void revokeToken(const std::string& token);
    
    // Secrets
    std::string tokenizeSecret(const std::string& rawSecret);
    std::string detokenizeSecret(const std::string& tokenId);

    // Sandbox
    SandboxManager& getSandboxManager() { return m_sandboxManager; }
    bool validateSandboxAction(const AgentHubConnector& conn, const std::string& action, const std::string& resource);

    // Security (AV & Compliance)
    bool scanPayload(const std::string& content, const std::string& sourceId, std::string& outReason);
    void quarantinePayload(const std::string& content, const std::string& source, const std::string& reason);
    std::vector<AgentQuarantineEntry> getQuarantineList();
    bool releaseQuarantine(const std::string& id);
    void deleteQuarantine(const std::string& id);
    
    // Audit & Logging
    void logAudit(const std::string& action, const std::string& component, const std::string& details, const std::string& status = "SUCCESS");
    std::vector<AgentAuditEntry> getAuditLog(int limit = 50);
    
    void configureScanner(const AgentScannerConfig& config);
    AgentLedger& getLedger() { return m_ledger; }
    void syncLedger(const std::vector<AgentLedgerBlock>& externalChain, const std::string& peerId);
    void disconnectLedger();
    void reconnectLedger();
    bool isLedgerIsolated() { std::lock_guard<std::mutex> l(m_mutex); return m_ledgerIsolated; }

    void addResource(const AgentResource& res);
    std::vector<AgentResource> getResources();
    void updateResourceLastPoll(const std::string& id);
    void addFirewallRule(const AgentFirewallRule& rule);
    bool validatePayload(const std::string& payload);
    bool validateEgress(const std::string& payload, const std::string& target);
    
    void addConnectionSecret(const AgentConnectionSecret& secret);
    void deleteConnectionSecret(const std::string& id);
    void groupConnectionSecret(const std::string& id, const std::string& groupName);
    void ungroupConnectionSecret(const std::string& id);
    std::vector<AgentConnectionSecret> getConnectionSecrets();
    
    bool checkSecretsLeak(const std::string& payload, const std::string& target, bool isControlPlane);
    void suspendConnector(const std::string& id);
    void resumeConnector(const std::string& id);
    bool isConnectorSuspended(const std::string& id);

    void blacklistEntity(const std::string& id);
    bool isBlacklisted(const std::string& id);
    void setState(std::unique_ptr<IAgentState> s) { std::lock_guard<std::mutex> l(m_mutex); m_state = std::move(s); }
    IAgentState* getState() { std::lock_guard<std::mutex> l(m_mutex); return m_state.get(); }
    void triggerAlert(const std::string& type, const std::string& agentId, const std::string& details);
    std::vector<AgentAlert> getRecentAlerts(int limit = 20);
    CryptoManager& getCrypto() { return m_crypto; }

private:
    std::mutex m_mutex;
    CryptoManager m_crypto;
    std::unique_ptr<IAgentState> m_state;
    std::vector<AgentMessage> m_messageHistory; 
    std::map<std::string, AgentResource> m_resources;
    std::vector<AgentFirewallRule> m_fwRules;
    std::set<std::string> m_blacklist;
    std::vector<AgentAlert> m_alerts;
    std::map<std::string, int> m_agentHealth;
    
    std::map<std::string, AgentHubConnector> m_connectors;
    std::set<std::string> m_suspendedConnectors;

    std::map<std::string, AgentSystemToken> m_tokens;
    std::map<std::string, std::string> m_vault;
    std::map<std::string, AgentConnectionSecret> m_connectionSecrets;

    std::map<std::string, AgentQuarantineEntry> m_quarantine;
    std::vector<AgentAuditEntry> m_auditLog;
    AgentScannerConfig m_scannerConfig;

    MalwareScannerChain m_scannerChain;
    std::vector<std::shared_ptr<IDataLeakDetector>> m_leakDetectors;

    AgentLedger m_ledger;
    std::string m_ledgerPath;
    bool m_ledgerIsolated = false;
    
    SandboxManager m_sandboxManager;

    void initializePaths() {
        auto sandboxPath = AgenticConfigManager::getInstance().getSandboxPath();
        m_ledgerPath = sandboxPath / "blockchain_ledger.enc";
        std::cerr << "[MODEL] Data path initialized to sandbox: " << m_ledgerPath << std::endl;
    }
};
