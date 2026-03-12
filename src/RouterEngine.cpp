#include "RouterEngine.hpp"
#include <httplib.h>
#include <iostream>
#include <fstream>
#include <map>

// Need a simple way to track message offsets per connector
static std::map<std::string, long long> m_lastMsgProcessed;
static std::map<std::string, std::streampos> m_fileOffsets;

RouterEngine::RouterEngine(AgenticPipelineModel& model) : m_model(model), m_running(false) {}
RouterEngine::~RouterEngine() { stop(); }

void RouterEngine::start() {
    m_running = true;
    m_loop = std::thread(&RouterEngine::run, this);
}

void RouterEngine::stop() {
    if (m_running) {
        m_running = false;
        if (m_loop.joinable()) m_loop.join();
    }
}

void RouterEngine::run() {
    while (m_running) {
        auto connectors = m_model.getConnectors();

        for (auto& conn : connectors) {
            if (!conn.active || m_model.isConnectorSuspended(conn.id) || !m_model.getState()->canConnect()) continue; 

            if (conn.strategy && conn.strategy->getType() == "LOCAL_FILE") {
                handleFileTail(conn);
            } else if (conn.strategy && conn.strategy->getType() == "PEER_MVC" ||
                       conn.strategy && conn.strategy->getType() == "URL_STREAM" ||
                       conn.strategy && conn.strategy->getType() == "FILE_SINK") {

                auto allMessages = m_model.getRecentMessages(50);
                std::vector<AgentMessage> pending;
                long long lastTs = m_lastMsgProcessed.count(conn.id) ? m_lastMsgProcessed[conn.id] : 0;

                bool connectionSuspended = false;

                for (const auto& m : allMessages) {
                    long long msgTs = std::chrono::duration_cast<std::chrono::milliseconds>(m.timestamp.time_since_epoch()).count();
                    if (msgTs > lastTs && m.isLocal) {
                        // 1. AGENT MALWARE SCAN (Real-Time Egress Protection)
                        std::string scanReason;
                        if (!m_model.scanPayload(m.content, conn.id, scanReason)) {
                            m_model.quarantinePayload(m.content, conn.id, scanReason);
                            continue;
                        }

                        // 2. SECRETS LEAK PREVENTION (Instant Connection Stop)
                        if (conn.strategy) {
                            conn.strategy->secureSend(conn, m_model, m.content);
                            if (m_model.isConnectorSuspended(conn.id)) {
                                connectionSuspended = true;
                                break;
                            }
                        }

                        // 3. AGENT LEAK PREVENTION (Regex Rules)
                        if (m_model.validateEgress(m.content, conn.target)) {
                            pending.push_back(m);
                        }
                    }
                }

                if (connectionSuspended) continue; // Connector was suspended mid-processing

                if (!pending.empty()) {                    if (conn.strategy && conn.strategy->getType() == "PEER_MVC") handlePeerSync(conn, pending);
                    else if (conn.strategy && conn.strategy->getType() == "FILE_SINK") handleFileWriter(conn, pending);
                    else if (conn.strategy && conn.strategy->getType() == "URL_STREAM") handleUrlStream(conn, pending);

                    m_lastMsgProcessed[conn.id] = std::chrono::duration_cast<std::chrono::milliseconds>(pending.back().timestamp.time_since_epoch()).count();
                }
            } else if (conn.strategy && conn.strategy->getType() == "SYSTEM_INTF") {
                handleSystemIntf(conn);
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void RouterEngine::handleFileTail(AgentHubConnector& conn) {
    if (!m_model.validateSandboxAction(conn, "DISK_READ", conn.target)) return;

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

        std::string scanReason;
        if (!m_model.scanPayload(line, "AgentFile_" + conn.id, scanReason)) {
            m_model.quarantinePayload(line, "AgentFile_" + conn.id, scanReason);
            continue;
        }

        AgentMessage msg;
        msg.senderId = "AgentFile_" + conn.id;
        msg.topic = "agent_file_stream";
        msg.content = line;
        msg.timestamp = std::chrono::system_clock::now();
        m_model.pushMessage(msg);
    }
    m_fileOffsets[conn.id] = file.tellg();
}

void RouterEngine::handleFileWriter(AgentHubConnector& conn, const std::vector<AgentMessage>& pending) {
    if (!m_model.validateSandboxAction(conn, "DISK_WRITE", conn.target)) return;

    std::ofstream file(conn.target, std::ios::app);
    if (!file.is_open()) return;

    for (const auto& m : pending) {
        std::string secureContent = m.content;
        if (secureContent.find("TOKENIZE:") == 0) {
            std::string raw = secureContent.substr(9);
            secureContent = m_model.tokenizeSecret(raw);
        }
        file << "[Agent:" << m.senderId << "] " << secureContent << std::endl;
    }
}

void RouterEngine::handleUrlStream(AgentHubConnector& conn, const std::vector<AgentMessage>& pending) {
    if (!m_model.validateSandboxAction(conn, "NET_CONNECT", conn.target)) return;

    for (const auto& m : pending) {
         try {
            httplib::Client cli(conn.target);
            cli.Post("/agent_webhook", m.content, "application/json");
         } catch (...) {}
    }
}

void RouterEngine::handlePeerSync(AgentHubConnector& conn, const std::vector<AgentMessage>& backlog) {
    try {
        std::string target = conn.target;
        if (target.find("http://") == 0) target = target.substr(7);
        
        size_t colon = target.find_last_of(':');
        if (colon == std::string::npos) return;

        std::string host = target.substr(0, colon);
        int port = std::stoi(target.substr(colon+1));

        httplib::Client cli(host, port);
        cli.set_connection_timeout(2, 0);
        cli.set_read_timeout(5, 0);
        
        json jBatch = json::array();
        for (const auto& m : backlog) {
            jBatch.push_back(m.toJson());
        }

        auto res = cli.Post("/api/send_batch", jBatch.dump(), "application/json");

        auto ledgerRes = cli.Get("/api/ledger");
        if (ledgerRes && ledgerRes->status == 200) {
            try {
                json jChain = json::parse(ledgerRes->body);
                std::vector<AgentLedgerBlock> externalChain;
                for (const auto& bJson : jChain) {
                    AgentLedgerBlock b;
                    b.index = bJson["index"];
                    b.timestamp = bJson["timestamp"];
                    b.data = bJson["data"];
                    b.prevHash = bJson["prev_hash"];
                    b.hash = bJson["hash"];
                    externalChain.push_back(b);
                }
                m_model.syncLedger(externalChain, conn.id);
            } catch (...) {}
        }
    } catch (...) {}
}

void RouterEngine::handleSystemIntf(AgentHubConnector& conn) {
    if (conn.metadata.contains("mac")) {
        AgentMessage msg;
        msg.senderId = "AgenticSysIntf";
        msg.topic = "intf_monitor";
        msg.content = "Agent Interface " + conn.target + " (MAC: " + conn.metadata["mac"].get<std::string>() + ") active.";
        msg.timestamp = std::chrono::system_clock::now();
        m_model.pushMessage(msg);
    }
}
