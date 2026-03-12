#include "Model.hpp"
#include "states/AgentStates.hpp"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <sstream>

json AgentQuarantineEntry::toJson() const {
    long long ts = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count();
    return json{{"id", id}, {"preview", contentPreview}, {"origin", origin}, {"reason", reason}, {"timestamp", ts}, {"resolved", resolved}};
}

json AgentAuditEntry::toJson() const {
    long long ts = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count();
    return json{{"id", id}, {"action", action}, {"component", component}, {"details", details}, {"status", status}, {"timestamp", ts}};
}

json AgentMessage::toJson() const {
    return json{{"sender", senderId}, {"topic", topic}, {"content", content}, {"encrypted", encrypted}, {"is_local", isLocal}, 
 {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count()}};
}

json AgentAlert::toJson() const {
    return json{{"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count()}, {"type", type}, {"agent", agentId}, {"details", details}};
}

/**
 * @brief Concrete implementation of IDataLeakDetector for cryptographic keys.
 */
class KeyLeakDetector : public IDataLeakDetector {
public:
    KeyLeakDetector(const std::map<std::string, AgentConnectionSecret>& secrets) : m_secrets(secrets) {}
    LeakDetection detect(const std::string& data, bool isControlPlane) override {
        if (isControlPlane) return { false, "" };
        for (const auto& [id, secret] : m_secrets) {
            if (!secret.keyData.empty() && data.find(secret.keyData) != std::string::npos) {
                return { true, "Connection secret '" + id + "' found in payload." };
            }
        }
        return { false, "" };
    }
    std::string getName() const override { return "KeyLeakDetector"; }
private:
    const std::map<std::string, AgentConnectionSecret>& m_secrets;
};

AgenticPipelineModel::AgenticPipelineModel() {
    m_state = std::make_unique<NormalState>();

    // Initialize Malware Scanner Chain (Strategy + Chain of Responsibility)
    m_scannerChain.addScanner(std::make_shared<ClamAvScanner>()); // Centralized/Default

    // Initialize Leak Detection Chain (Strategy Pattern)
    m_leakDetectors.push_back(std::make_shared<KeyLeakDetector>(m_connectionSecrets));

    initializePaths();
    // Load existing private ledger blockchain state
    m_ledger.loadFromFile(m_ledgerPath, &m_crypto);
    
    // Replay ledger transactions to reconstruct state securely
    for (const auto& block : m_ledger.getChain()) {
        if (!block.data.is_object()) continue;
        std::string type = block.data.value("type", "");
        
        if (type == "CONNECTION_SECRET_ADD") {
            AgentConnectionSecret s;
            s.id = block.data.value("secret_id", "");
            s.type = block.data.value("secret_type", "");
            std::string encryptedKey = block.data.value("encrypted_key", "");
            if (!encryptedKey.empty()) {
                s.keyData = m_crypto.decrypt(encryptedKey);
            }
            s.groupName = block.data.value("group_name", "");
            s.isControlPlane = block.data.value("is_control_plane", false);
            m_connectionSecrets[s.id] = s;
        } else if (type == "CONNECTION_SECRET_DEL") {
            m_connectionSecrets.erase(block.data.value("secret_id", ""));
        } else if (type == "CONNECTION_SECRET_GROUP") {
            std::string id = block.data.value("secret_id", "");
            if (m_connectionSecrets.find(id) != m_connectionSecrets.end()) {
                m_connectionSecrets[id].groupName = block.data.value("group_name", "");
            }
        } else if (type == "CONNECTION_SECRET_UNGROUP") {
            std::string id = block.data.value("secret_id", "");
            if (m_connectionSecrets.find(id) != m_connectionSecrets.end()) {
                m_connectionSecrets[id].groupName = "";
            }
        }
    }
}

void AgenticPipelineModel::pushMessage(const AgentMessage& msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_messageHistory.push_back(msg);
    if (m_messageHistory.size() > 500) m_messageHistory.erase(m_messageHistory.begin());
}

std::vector<AgentMessage> AgenticPipelineModel::getRecentMessages(int limit) {
    std::lock_guard<std::mutex> lock(m_mutex);
    int count = std::min((int)m_messageHistory.size(), limit);
    if (count == 0) return {};
    return std::vector<AgentMessage>(m_messageHistory.end() - count, m_messageHistory.end());
}

// Distributed Agent Hub
void AgenticPipelineModel::addConnector(const AgentHubConnector& conn) {    std::lock_guard<std::mutex> lock(m_mutex);
    m_connectors[conn.id] = conn;
}

std::vector<AgentHubConnector> AgenticPipelineModel::getConnectors() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<AgentHubConnector> res;
    for (const auto& [id, c] : m_connectors) res.push_back(c);
    return res;
}

void AgenticPipelineModel::removeConnector(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connectors.erase(id);
}

// --- AGENT TOKENIZATION ENGINE IMPLEMENTATION ---

std::string AgenticPipelineModel::issueToken(const std::string& ownerId, AgentTokenScope scope, int ttlMinutes) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Simple secure token generation (Agentic Token Strategy)
    std::string charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string token = (scope == AgentTokenScope::INTERNAL ? "INT_" : (scope == AgentTokenScope::EXTERNAL ? "EXT_" : "GLB_"));
    
    for (int i = 0; i < 32; i++) {
        token += charset[rand() % charset.length()];
    }

    AgentSystemToken t;
    t.token = token;
    t.ownerId = ownerId;
    t.scope = scope;
    t.expiry = std::chrono::system_clock::now() + std::chrono::minutes(ttlMinutes);
    t.active = true;

    m_tokens[token] = t;
    std::cout << "[AGENT_TOKEN] Issued " << (int)scope << " token for " << ownerId << " -> " << token.substr(0, 8) << "..." << std::endl;

    // --- RECORD TO PRIVATE AGENT LEDGER ---
    json tx = {{"type", "TOKEN_ISSUE"}, {"owner", ownerId}, {"scope", (int)scope}, {"token_preview", token.substr(0, 8)}};
    m_ledger.addBlock(tx);
    m_ledger.saveToFile(m_ledgerPath, &m_crypto);

    return token;
}

bool AgenticPipelineModel::validateToken(const std::string& token, AgentTokenScope requiredScope) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_tokens.find(token) == m_tokens.end()) return false;
    
    const auto& t = m_tokens[token];
    if (!t.active) return false;
    if (std::chrono::system_clock::now() > t.expiry) {
        std::cout << "[AGENT_TOKEN] Expired token used: " << token.substr(0, 8) << "..." << std::endl;
        return false;
    }

    // Scoped Access: INTERNAL can do anything, EXTERNAL/GLOBAL must match or be subset
    if (t.scope == AgentTokenScope::INTERNAL) return true;
    return t.scope == requiredScope;
}

void AgenticPipelineModel::revokeToken(const std::string& token) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_tokens.find(token) != m_tokens.end()) {
        m_tokens[token].active = false;
        std::cout << "[AGENT_TOKEN] Revoked: " << token.substr(0, 8) << "..." << std::endl;
    }
}

std::string AgenticPipelineModel::tokenizeSecret(const std::string& rawSecret) {
    // Replaces sensitive data with a reference token, storing it in the encrypted vault
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string tokenId = "VAULT_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    
    // Encrypt before storing in vault (Agentic Secure Storage)
    std::string encrypted = m_crypto.encrypt(rawSecret);
    m_vault[tokenId] = encrypted;

    // --- RECORD TO PRIVATE AGENT LEDGER ---
    json tx = {{"type", "VAULT_TOKENIZE"}, {"token_id", tokenId}};
    m_ledger.addBlock(tx);
    m_ledger.saveToFile(m_ledgerPath, &m_crypto);

    std::cout << "[AGENT_VAULT] Tokenized secret into " << tokenId << std::endl;
    return tokenId;
}

std::string AgenticPipelineModel::detokenizeSecret(const std::string& tokenId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_vault.find(tokenId) == m_vault.end()) return "INVALID_TOKEN";
    
    return m_crypto.decrypt(m_vault[tokenId]);
}

bool AgenticPipelineModel::validateSandboxAction(const AgentHubConnector& conn, const std::string& action, const std::string& resource) {
    if (conn.sandbox.level == AgentSandboxLevel::NONE) return true;

    bool allowed = false;
    std::string reason = "Unknown restriction";

    if (action == "DISK_READ" || action == "DISK_WRITE") {
        if (action == "DISK_WRITE" && !conn.sandbox.allowDiskWrite) {
            reason = "Disk write globally disabled for this connector";
        } else {
            // Path Prefix Validation
            for (const auto& path : conn.sandbox.allowedPaths) {
                if (resource.find(path) == 0) { allowed = true; break; }
            }
            if (!allowed) reason = "Path " + resource + " not in allowed sandbox list";
        }
    } else if (action == "NET_CONNECT") {
        if (!conn.sandbox.allowNetwork) {
            reason = "Network access globally disabled for this connector";
        } else {
            // Domain/URL Validation
            for (const auto& domain : conn.sandbox.allowedDomains) {
                if (resource.find(domain) != std::string::npos) { allowed = true; break; }
            }
            if (!allowed) reason = "Domain " + resource + " not in allowed sandbox list";
        }
    }

    if (!allowed && conn.sandbox.level != AgentSandboxLevel::NONE) {
        std::cout << "[AGENT_SANDBOX] BLOCK: Connector '" << conn.id << "' attempted " << action << " on " << resource << ". Reason: " << reason << std::endl;
        triggerAlert("SANDBOX_VIOLATION", conn.id, "Attempted unauthorized " + action + " on " + resource);
        return false;
    }

    return true;
}

// --- REAL-TIME AGENT SECURITY IMPLEMENTATION ---

void AgenticPipelineModel::configureScanner(const AgentScannerConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_scannerConfig = config;
    
    // Add user configured scanner to the strategy chain
    if (!config.customCmd.empty()) {
        m_scannerChain.addScanner(std::make_shared<UserConfigScanner>(config.customCmd));
    }
    
    std::cout << "[AGENT_SECURITY] AV Scanner Chain Updated. Custom Command: " << (config.customCmd.empty() ? "NONE" : config.customCmd) << std::endl;
}

bool AgenticPipelineModel::scanPayload(const std::string& content, const std::string& sourceId, std::string& outReason) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_scannerConfig.enabled) return true;

    // Execute Strategy + Chain of Responsibility
    MalwareInfo info = m_scannerChain.executeScan(content);

    if (info.result == ScanResult::NOT_FOUND) {
        outReason = "CRITICAL: No malware scanner available.";
        triggerAlert("SECURITY_GAP", sourceId, outReason);
        
        // Immediate connection suspension as mandated
        if (!sourceId.empty()) {
            m_suspendedConnectors.insert(sourceId);
            std::cerr << "[AGENT_SECURITY] ALERT: Suspending connection '" << sourceId << "' due to missing AV scanner." << std::endl;
        }
        return false;
    }

    if (info.result == ScanResult::INFECTED) {
        outReason = info.detail;
        triggerAlert("MALWARE_DETECTED", sourceId, "Malware detected by " + info.scannerName + ": " + info.detail);
        
        // Immediate connection suspension as mandated
        if (!sourceId.empty()) {
            m_suspendedConnectors.insert(sourceId);
            std::cerr << "[AGENT_SECURITY] CRITICAL: Suspending connection '" << sourceId << "' due to malware detection." << std::endl;
        }
        return false;
    }

    if (info.result == ScanResult::ERROR) {
        outReason = "Scanner Error: " + info.detail;
        return !m_scannerConfig.blockOnError; // Fail closed if configured
    }

    return true; // CLEAN
}

void AgenticPipelineModel::quarantinePayload(const std::string& content, const std::string& source, const std::string& reason) {
    std::lock_guard<std::mutex> lock(m_mutex);
    AgentQuarantineEntry q;
    q.id = "Q_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    q.origin = source;
    q.reason = reason;
    q.timestamp = std::chrono::system_clock::now();
    q.contentPreview = content.substr(0, std::min((size_t)50, content.length()));
    q.encryptedContent = m_crypto.encrypt(content); // Store securely
    
    m_quarantine[q.id] = q;
    logAudit("QUARANTINE", source, "Moved unauthorized payload to " + q.id + ": " + reason, "FLAGGED");
    std::cout << "[AGENT_SECURITY] QUARANTINED " << q.id << " from " << source << ". Reason: " << reason << std::endl;
}

std::vector<AgentQuarantineEntry> AgenticPipelineModel::getQuarantineList() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<AgentQuarantineEntry> list;
    for (const auto& kv : m_quarantine) list.push_back(kv.second);
    return list;
}

bool AgenticPipelineModel::releaseQuarantine(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_quarantine.find(id) == m_quarantine.end()) return false;
    
    m_quarantine[id].resolved = true;
    logAudit("RELEASE", "Admin", "Released payload " + id + " from quarantine");
    return true;
}

void AgenticPipelineModel::deleteQuarantine(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_quarantine.erase(id);
    logAudit("DELETE", "Admin", "Destroyed quarantined payload " + id);
}

void AgenticPipelineModel::logAudit(const std::string& action, const std::string& component, const std::string& details, const std::string& status) {
    AgentAuditEntry a;
    a.id = "LOG_" + std::to_string(m_auditLog.size() + 1);
    a.action = action;
    a.component = component;
    a.details = details;
    a.status = status;
    a.timestamp = std::chrono::system_clock::now();
    
    m_auditLog.push_back(a);
    if (m_auditLog.size() > 1000) m_auditLog.erase(m_auditLog.begin());
}

std::vector<AgentAuditEntry> AgenticPipelineModel::getAuditLog(int limit) {
    std::lock_guard<std::mutex> lock(m_mutex);
    int start = std::max(0, (int)m_auditLog.size() - limit);
    return std::vector<AgentAuditEntry>(m_auditLog.begin() + start, m_auditLog.end());
}

void AgenticPipelineModel::syncLedger(const std::vector<AgentLedgerBlock>& externalChain, const std::string& peerId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // EMERGENCY KILL SWITCH: Block all syncing if isolated
    if (m_ledgerIsolated) {
        logAudit("SYNC_REJECTED", peerId, "Rejected ledger sync: System is in EMERGENCY ISOLATION", "WARNING");
        return;
    }

    // 1. SECURITY: Scan the incoming chain data for unauthorized code/signatures
    std::string chainDataString;
    for (const auto& b : externalChain) chainDataString += b.data.dump();
    
    std::string scanReason;
    if (!scanPayload(chainDataString, "Sync:" + peerId, scanReason)) {
        quarantinePayload(chainDataString, "Sync:" + peerId, "Unauthorized Blockchain Data: " + scanReason);
        logAudit("SYNC_BLOCK", peerId, "Blocked unauthorized ledger sync", "FLAGGED");
        return;
    }

    size_t oldSize = m_ledger.getChain().size();
    m_ledger.mergeChain(externalChain);
    
    if (m_ledger.getChain().size() > oldSize) {
        m_ledger.saveToFile(m_ledgerPath, &m_crypto);
        logAudit("LEDGER_SYNC", peerId, "Synchronized ledger. Growth: +" + std::to_string(m_ledger.getChain().size() - oldSize) + " blocks");
        std::cout << "[AGENT_SECURITY] Verified and Merged External Blockchain from " << peerId << std::endl;
    }
}

void AgenticPipelineModel::disconnectLedger() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_ledgerIsolated = true;
    
    logAudit("KILL_SWITCH", "System", "Private Agent Ledger has been ISOLATED from the network.", "CRITICAL");
    
    if (m_state->getName() != "LOCKDOWN") {
        m_state = std::make_unique<LockdownState>();
        triggerAlert("EMERGENCY_ISOLATION", "AgenticOrchestrator", "Blockchain sync disconnected due to security request.");
    }
    
    std::cout << "[AGENT_SECURITY] CRITICAL: Agent Ledger Disconnected and Isolated." << std::endl;
}

void AgenticPipelineModel::reconnectLedger() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_ledgerIsolated = false;
    
    logAudit("RECONNECT", "System", "Private Agent Ledger has been RECONNECTED to the network.", "SUCCESS");
    
    if (m_state->getName() == "LOCKDOWN") {
        m_state = std::make_unique<NormalState>();
    }
    
    std::cout << "[AGENT_SECURITY] Agent Ledger reconnected to network." << std::endl;
}

// Existing Logic Refactored
void AgenticPipelineModel::addResource(const AgentResource& res) { 
    std::lock_guard<std::mutex> lock(m_mutex); 
    m_resources[res.id] = res; 
    m_resources[res.id].lastPoll = std::chrono::system_clock::from_time_t(0); 
}

std::vector<AgentResource> AgenticPipelineModel::getResources() { 
    std::lock_guard<std::mutex> lock(m_mutex); 
    std::vector<AgentResource> res; 
    for (const auto& [id, r] : m_resources) res.push_back(r); 
    return res; 
}

void AgenticPipelineModel::updateResourceLastPoll(const std::string& id) { 
    std::lock_guard<std::mutex> lock(m_mutex); 
    if (m_resources.find(id) != m_resources.end()) m_resources[id].lastPoll = std::chrono::system_clock::now(); 
}

void AgenticPipelineModel::addFirewallRule(const AgentFirewallRule& rule) { 
    std::lock_guard<std::mutex> lock(m_mutex); 
    m_fwRules.push_back(rule); 
}

bool AgenticPipelineModel::validatePayload(const std::string& payload) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& rule : m_fwRules) {
        try {
            if (std::regex_search(payload, std::regex(rule.pattern, std::regex_constants::icase))) {
                std::cout << "[AGENT_SECURITY] Blocked Ingress Data Matching Rule: " << rule.id << std::endl;
                return false;
            }
        } catch (...) {}
    }
    return true;
}

bool AgenticPipelineModel::validateEgress(const std::string& payload, const std::string& target) {
    std::lock_guard<std::mutex> lock(m_mutex);
    bool allowed = true;
    for (const auto& rule : m_fwRules) {
        try {
            if (std::regex_search(payload, std::regex(rule.pattern, std::regex_constants::icase))) {
                AgentAlert a;
                a.timestamp = std::chrono::system_clock::now();
                a.type = "DATA_LEAK_PREVENTED";
                a.agentId = "AgenticDLP";
                a.details = "Blocked attempt to send secret matching rule '" + rule.id + "' to " + target;
                m_alerts.push_back(a);
                std::cout << "[AGENT_SECURITY] CRITICAL: Stopped leak to " << target << " matching pattern " << rule.id << std::endl;
                allowed = false;
                break;
            }
        } catch (...) {}
    }
    return allowed;
}

// --- Connection Secrets Management ---
void AgenticPipelineModel::addConnectionSecret(const AgentConnectionSecret& secret) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connectionSecrets[secret.id] = secret;
    
    json tx = {
        {"type", "CONNECTION_SECRET_ADD"},
        {"secret_id", secret.id},
        {"secret_type", secret.type},
        {"encrypted_key", m_crypto.encrypt(secret.keyData)}, // Highest security standard
        {"group_name", secret.groupName},
        {"is_control_plane", secret.isControlPlane}
    };
    m_ledger.addBlock(tx);
    m_ledger.saveToFile(m_ledgerPath, &m_crypto);
    
    std::cout << "[AGENT_SECURITY] Added connection secret/key to private blockchain: " << secret.id << " (Type: " << secret.type << ")" << std::endl;
}

void AgenticPipelineModel::deleteConnectionSecret(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connectionSecrets.erase(id);
    
    json tx = {
        {"type", "CONNECTION_SECRET_DEL"},
        {"secret_id", id}
    };
    m_ledger.addBlock(tx);
    m_ledger.saveToFile(m_ledgerPath, &m_crypto);
    
    std::cout << "[AGENT_SECURITY] Deleted connection secret/key from private blockchain: " << id << std::endl;
}

void AgenticPipelineModel::groupConnectionSecret(const std::string& id, const std::string& groupName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_connectionSecrets.find(id) != m_connectionSecrets.end()) {
        m_connectionSecrets[id].groupName = groupName;
        
        json tx = {
            {"type", "CONNECTION_SECRET_GROUP"},
            {"secret_id", id},
            {"group_name", groupName}
        };
        m_ledger.addBlock(tx);
        m_ledger.saveToFile(m_ledgerPath, &m_crypto);
    }
}

void AgenticPipelineModel::ungroupConnectionSecret(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_connectionSecrets.find(id) != m_connectionSecrets.end()) {
        m_connectionSecrets[id].groupName = "";
        
        json tx = {
            {"type", "CONNECTION_SECRET_UNGROUP"},
            {"secret_id", id}
        };
        m_ledger.addBlock(tx);
        m_ledger.saveToFile(m_ledgerPath, &m_crypto);
    }
}

std::vector<AgentConnectionSecret> AgenticPipelineModel::getConnectionSecrets() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<AgentConnectionSecret> res;
    for (const auto& [id, s] : m_connectionSecrets) res.push_back(s);
    return res;
}

bool AgenticPipelineModel::checkSecretsLeak(const std::string& payload, const std::string& target, bool isControlPlane) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto& detector : m_leakDetectors) {
        LeakDetection result = detector->detect(payload, isControlPlane);
        if (result.leaked) {
            AgentAlert a;
            a.timestamp = std::chrono::system_clock::now();
            a.type = "DATA_LEAK_PREVENTED";
            a.agentId = detector->getName();
            a.details = "CRITICAL: " + result.detail + " Connection to " + target + " suspended for user review (Resume or Terminate).";
            m_alerts.push_back(a);
            
            std::cerr << "[AGENT_SECURITY] " << a.details << std::endl;
            
            // Auto-suspend: find the connector that matches the target (simplified)
            for (auto& [id, conn] : m_connectors) {
                if (conn.target == target) {
                    m_suspendedConnectors.insert(id);
                    conn.suspended = true;
                }
            }
            return false;
        }
    }
    return true; // No leak detected
}

void AgenticPipelineModel::suspendConnector(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_suspendedConnectors.insert(id);
    if (m_connectors.find(id) != m_connectors.end()) {
        m_connectors[id].suspended = true;
    }
    std::cout << "[AGENT_SECURITY] Connector suspended: " << id << std::endl;
}

void AgenticPipelineModel::resumeConnector(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_suspendedConnectors.erase(id);
    if (m_connectors.find(id) != m_connectors.end()) {
        m_connectors[id].suspended = false;
    }
    std::cout << "[AGENT_SECURITY] Connector resumed: " << id << std::endl;
}

bool AgenticPipelineModel::isConnectorSuspended(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_suspendedConnectors.count(id) > 0 || (m_connectors.count(id) && m_connectors[id].suspended);
}

void AgenticPipelineModel::blacklistEntity(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex); 
    m_blacklist.insert(id); 
}

bool AgenticPipelineModel::isBlacklisted(const std::string& id) { 
    std::lock_guard<std::mutex> lock(m_mutex); 
    return m_blacklist.count(id) > 0; 
}

void AgenticPipelineModel::triggerAlert(const std::string& type, const std::string& agentId, const std::string& details) {
    std::lock_guard<std::mutex> lock(m_mutex);
    AgentAlert a; 
    a.timestamp = std::chrono::system_clock::now(); 
    a.type = type; 
    a.agentId = agentId; 
    a.details = details;
    m_alerts.push_back(a); 
    m_agentHealth[agentId]++;
    if (m_agentHealth[agentId] >= 3) m_blacklist.insert(agentId);
    if (m_alerts.size() > 100) m_alerts.erase(m_alerts.begin());
}

std::vector<AgentAlert> AgenticPipelineModel::getRecentAlerts(int limit) {
    std::lock_guard<std::mutex> lock(m_mutex);
    int count = std::min((int)m_alerts.size(), limit);
    if (count == 0) return {};
    return std::vector<AgentAlert>(m_alerts.end() - count, m_alerts.end());
}
