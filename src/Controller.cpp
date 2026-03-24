#include "Controller.hpp"
#include "connectors/ConnectorStrategies.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

AgenticHttpController::AgenticHttpController(AgenticPipelineModel& model, int port) : m_model(model), m_port(port), m_running(false) {
    setupRoutes();
}

AgenticHttpController::~AgenticHttpController() { stop(); }

void AgenticHttpController::start() {
    m_wayland.start();
    m_running = true;
    std::cout << "[HTTP] Agentic Gateway starting in BACKGROUND on 127.0.0.1:" << m_port << "..." << std::endl;
    m_serverThread = std::thread([this]() {
        if (!m_svr.listen("127.0.0.1", m_port)) {
            std::cerr << "[HTTP] FATAL: Gateway failed to listen on 127.0.0.1:" << m_port << std::endl;
        }
    });
}

void AgenticHttpController::stop() {
    if (m_running) { m_running = false; m_svr.stop(); m_wayland.stop(); if (m_serverThread.joinable()) m_serverThread.join(); }
}

void AgenticHttpController::setupRoutes() {
    m_svr.set_logger([this](const httplib::Request& req, const httplib::Response& res) {
        std::string detail = "";
        if (req.method == "POST") {
            if (req.path == "/api/send") {
                try {
                    auto j = json::parse(req.body);
                    detail = " from " + j.at("sender").get<std::string>() + " (topic: " + j.at("topic").get<std::string>() + ")";
                } catch(...) {}
            } else if (req.path == "/api/send_batch") {
                try {
                    auto j = json::parse(req.body);
                    if (j.is_array()) detail = " [BATCH: " + std::to_string(j.size()) + " msgs]";
                } catch(...) {}
            }
        }
        std::cout << "[HTTP_GATEWAY] " << req.method << " " << req.path << detail << " -> " << res.status << " (Port: " << m_port << ")" << std::endl;
    });

    m_svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::ifstream f((std::filesystem::current_path() / "www" / "index.html").string());
        if (f) { std::stringstream b; b << f.rdbuf(); res.set_content(b.str(), "text/html"); }
    });

    // --- SECURE AGENT MESSAGE HANDLER WITH AFW ---
    m_svr.Post("/api/send", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string sender = j.at("sender");
            std::string topic = j.at("topic");
            std::string content = j.at("content");

            // 1. Check Global Lockdown
            if (!m_model.getState()->handleRequest()) {
                res.status = 503; res.set_content("SYSTEM_LOCKDOWN", "text/plain"); return;
            }

            // 2. Check Blacklist
            if (m_model.isBlacklisted(sender)) {
                res.status = 401; res.set_content("BLACKLISTED", "text/plain"); return;
            }

            // --- INGRESS MALWARE SCAN ---
            std::string scanReason;
            if (!m_model.scanPayload(content, sender, scanReason)) {
                m_model.quarantinePayload(content, sender, scanReason);
                res.status = 403;
                res.set_content("AGENT_SECURITY_BLOCK: Unauthorized content detected", "text/plain");
                return;
            }

            // 3. Application Firewall: Payload Inspection
            if (!m_model.validatePayload(content)) {
                m_model.triggerAlert("AFW_BLOCK", sender, "Unauthorized agent payload pattern detected.");
                res.status = 403; res.set_content("UNAUTHORIZED_AGENT_CONTENT_BLOCKED", "text/plain"); return;
            }

            // Process message
            AgentMessage msg; msg.senderId = sender; msg.topic = topic; msg.content = content;
            msg.isLocal = true; // Received from local agent
            msg.timestamp = std::chrono::system_clock::now();
            m_model.pushMessage(msg);
            res.set_content("OK", "text/plain");
        } catch (...) { res.status = 400; }
    });

    m_svr.Post("/api/send_batch", [this](const httplib::Request& req, httplib::Response& res) {
        // --- RAW INGRESS SCAN (Before Parsing) ---
        std::string rawReason;
        if (!m_model.scanPayload(req.body, "IngressGate", rawReason)) {
             res.status = 403;
             res.set_content("Security Violation: " + rawReason, "text/plain");
             return;
        }

        try {
            auto jBatch = json::parse(req.body);
            if (!jBatch.is_array()) { res.status = 400; return; }

            int count = 0;
            for (const auto& j : jBatch) {
                std::string sender = j.at("sender");
                std::string topic = j.at("topic");
                std::string content = j.at("content");

                if (m_model.isBlacklisted(sender)) continue;

                // --- INGRESS MALWARE SCAN ---
                std::string scanReason;
                if (!m_model.scanPayload(content, sender, scanReason)) {
                    m_model.quarantinePayload(content, sender, scanReason);
                    continue;
                }

                if (!m_model.validatePayload(content)) {
                    m_model.triggerAlert("AFW_BLOCK_BATCH", sender, "Unauthorized payload in agent batch.");
                    continue;
                }

                AgentMessage msg; msg.senderId = sender; msg.topic = topic; msg.content = content;
                msg.isLocal = false; // Received from peer agent
                msg.timestamp = std::chrono::system_clock::now();
                m_model.pushMessage(msg);
                count++;
            }
            res.set_content("Processed " + std::to_string(count), "text/plain");
        } catch (...) { res.status = 400; }
    });

    // --- SANDBOX MANAGEMENT ENDPOINTS ---
    m_svr.Get("/api/sandbox", [this](const httplib::Request&, httplib::Response& res) {
        auto profiles = m_model.getSandboxManager().listSandboxes();
        json jList = json::array();
        for (const auto& p : profiles) jList.push_back(p.toJson());
        res.set_content(jList.dump(), "application/json");
    });

    m_svr.Get("/api/hub/:id", [this](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.path_params.at("id");
        auto connectors = m_model.getConnectors();
        auto it = std::find_if(connectors.begin(), connectors.end(), [&](const AgentHubConnector& c) {
            return c.id == id;
        });

        if (it != connectors.end()) {
            json j = {
                {"id", it->id},
                {"type", it->strategy ? it->strategy->getType() : "UNKNOWN"},
                {"target", it->target},
                {"active", it->active},
                {"suspended", it->suspended},
                {"isControlPlane", it->isControlPlane},
                {"sandbox", m_model.getSandboxManager().getSandbox(it->sandbox.id).toJson()}
            };
            res.set_content(j.dump(), "application/json");
        } else {
            res.status = 404;
        }
    });

    m_svr.Post("/api/sandbox", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            AgentSandboxProfile profile = AgentSandboxProfile::fromJson(j);
            
            // Check if exists
            auto existing = m_model.getSandboxManager().getSandbox(profile.id);
            if (existing.id.empty()) {
                m_model.getSandboxManager().createSandbox(profile);
                res.set_content("Sandbox Profile Created", "text/plain");
            } else {
                m_model.getSandboxManager().updateSandbox(profile);
                res.set_content("Sandbox Profile Updated", "text/plain");
            }
        } catch (...) { res.status = 400; }
    });

    m_svr.Delete("/api/sandbox", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string id = j.at("id");
            if (m_model.getSandboxManager().deleteSandbox(id)) {
                res.set_content("Sandbox Profile Deleted", "text/plain");
            } else {
                res.status = 404;
                res.set_content("Sandbox Profile Not Found", "text/plain");
            }
        } catch (...) { res.status = 400; }
    });

    m_svr.Post("/api/sandbox/revert", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string id = j.at("id");
            int version = j.at("version");
            if (m_model.getSandboxManager().revertSandbox(id, version)) {
                res.set_content("Sandbox Profile Reverted", "text/plain");
            } else {
                res.status = 400;
                res.set_content("Revert failed", "text/plain");
            }
        } catch (...) { res.status = 400; }
    });

    // Agent Hub/Router API
    m_svr.Get("/api/hub", [this](const httplib::Request&, httplib::Response& res) {
        auto connectors = m_model.getConnectors();
        json j = json::array();
        for (const auto& c : connectors) {
            j.push_back({
                {"id", c.id},
                {"type", c.strategy ? c.strategy->getType() : "UNKNOWN"},
                {"target", c.target},
                {"metadata", c.metadata},
                {"active", c.active}
            });
        }
        res.set_content(j.dump(), "application/json");
    });

    m_svr.Post("/api/hub", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            AgentHubConnector c;
            c.id = j.at("id");
            if (j.at("type").is_number()) {
                int typeInt = j.at("type");
                if (typeInt == 0) c.strategy = std::make_unique<PeerMvcStrategy>();
                else if (typeInt == 1) c.strategy = std::make_unique<LocalFileStrategy>();
                else if (typeInt == 2) c.strategy = std::make_unique<UrlStreamStrategy>();
                else if (typeInt == 3) c.strategy = std::make_unique<SystemIntfStrategy>();
                else if (typeInt == 4) c.strategy = std::make_unique<FileSinkStrategy>();
                else c.strategy = std::make_unique<PeerMvcStrategy>();
            } else if (j.at("type").is_string()) {
                std::string t = j.at("type");
                if (t == "PEER_MVC") c.strategy = std::make_unique<PeerMvcStrategy>();
                else if (t == "LOCAL_FILE") c.strategy = std::make_unique<LocalFileStrategy>();
                else if (t == "URL_STREAM") c.strategy = std::make_unique<UrlStreamStrategy>();
                else if (t == "SYSTEM_INTF") c.strategy = std::make_unique<SystemIntfStrategy>();
                else if (t == "FILE_SINK") c.strategy = std::make_unique<FileSinkStrategy>();
                else c.strategy = std::make_unique<PeerMvcStrategy>();
            }
            c.target = j.at("target");
            c.metadata = j.contains("metadata") ? j.at("metadata") : json::object();
            
            // NEW: Parse Agent Sandbox Profile
            if (j.contains("sandbox")) {
                auto js = j["sandbox"];
                c.sandbox.level = (AgentSandboxLevel)js.value("level", 0);
                if (js.contains("allowed_paths")) c.sandbox.allowedPaths = js["allowed_paths"].get<std::vector<std::string>>();
                if (js.contains("allowed_domains")) c.sandbox.allowedDomains = js["allowed_domains"].get<std::vector<std::string>>();
                c.sandbox.allowNetwork = js.value("allow_network", true);
                c.sandbox.allowDiskWrite = js.value("allow_disk_write", false);
            }

            m_model.addConnector(c);
            res.set_content("Agent Connector Added with Sandbox", "text/plain");
        } catch (...) { res.status = 400; }
    });

    m_svr.Post("/api/firewall", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            AgentFirewallRule r;
            r.id = j.at("id");
            r.pattern = j.at("pattern");
            r.blockRequest = j.value("block", true);
            m_model.addFirewallRule(r);
            std::cout << "[AFW] Added Agent Blocking Rule: " << r.id << std::endl;
            res.set_content("Rule Added", "text/plain");
        } catch (...) { res.status = 400; }
    });

    m_svr.Get("/api/state", [this](const httplib::Request&, httplib::Response& res) {
        json j;
        j["state"] = m_model.getState()->getName();
        j["isolated"] = m_model.isLedgerIsolated(); 
        
        auto msgs = m_model.getRecentMessages(20);
        json jMsgs = json::array();
        for (const auto& m : msgs) jMsgs.push_back(m.toJson());
        j["messages"] = jMsgs;

        auto alerts = m_model.getRecentAlerts(10);
        json jAlerts = json::array();
        for (const auto& a : alerts) jAlerts.push_back(a.toJson());
        j["alerts"] = jAlerts;

        res.set_content(j.dump(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
    });

    // --- AGENT TOKENIZATION SYSTEM ENDPOINTS ---
    m_svr.Post("/api/token/issue", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string owner = j.at("owner");
            AgentTokenScope scope = (AgentTokenScope)j.value("scope", 1); 
            int ttl = j.value("ttl", 60);
            
            std::string token = m_model.issueToken(owner, scope, ttl);
            res.set_content(json{{"token", token}, {"owner", owner}}.dump(), "application/json");
        } catch (...) { res.status = 400; }
    });

    m_svr.Post("/api/token/vault", [this](const httplib::Request& req, httplib::Response& res) {
        std::string auth = req.get_header_value("Authorization");
        if (auth.find("Bearer ") == 0) auth = auth.substr(7);
        
        if (!m_model.validateToken(auth, AgentTokenScope::INTERNAL)) {
            res.status = 401; res.set_content("UNAUTHORIZED", "text/plain"); return;
        }

        try {
            auto j = json::parse(req.body);
            if (j.contains("secret")) {
                std::string tokenId = m_model.tokenizeSecret(j["secret"]);
                res.set_content(json{{"token_id", tokenId}}.dump(), "application/json");
            } else if (j.contains("token_id")) {
                std::string raw = m_model.detokenizeSecret(j["token_id"]);
                res.set_content(json{{"secret", raw}}.dump(), "application/json");
            }
        } catch (...) { res.status = 400; }
    });

    // --- AGENT QUARANTINE & AUDIT ENDPOINTS ---
    m_svr.Get("/api/quarantine", [this](const httplib::Request&, httplib::Response& res) {
        auto list = m_model.getQuarantineList();
        json jList = json::array();
        for (const auto& q : list) jList.push_back(q.toJson());
        res.set_content(jList.dump(), "application/json");
    });

    m_svr.Post("/api/quarantine/manage", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string id = j.at("id");
            std::string action = j.at("action"); // "release" or "delete"
            
            if (action == "release") {
                if (m_model.releaseQuarantine(id)) res.set_content("Released", "text/plain");
                else res.status = 404;
            } else if (action == "delete") {
                m_model.deleteQuarantine(id);
                res.set_content("Deleted", "text/plain");
            } else {
                res.status = 400;
            }
        } catch (...) { res.status = 400; }
    });

    m_svr.Get("/api/audit", [this](const httplib::Request&, httplib::Response& res) {
        auto log = m_model.getAuditLog();
        json jLog = json::array();
        for (const auto& a : log) jLog.push_back(a.toJson());
        res.set_content(jLog.dump(), "application/json");
    });

    m_svr.Get("/api/ledger", [this](const httplib::Request&, httplib::Response& res) {
        auto chain = m_model.getLedger().getChain();
        json jChain = json::array();
        for (const auto& b : chain) jChain.push_back(b.toJson());
        res.set_content(jChain.dump(), "application/json");
    });

    m_svr.Post("/api/security/killswitch", [this](const httplib::Request&, httplib::Response& res) {
        m_model.disconnectLedger();
        res.set_content("AGENT_LEDGER_ISOLATED", "text/plain");
    });

    m_svr.Post("/api/security/reconnect", [this](const httplib::Request&, httplib::Response& res) {
        m_model.reconnectLedger();
        res.set_content("AGENT_LEDGER_RECONNECTED", "text/plain");
    });

    // --- CONNECTION SECRETS MANAGEMENT ---
    m_svr.Get("/api/security/secrets", [this](const httplib::Request&, httplib::Response& res) {
        auto secrets = m_model.getConnectionSecrets();
        json jList = json::array();
        for (const auto& s : secrets) jList.push_back(s.toJson());
        res.set_content(jList.dump(), "application/json");
    });

    m_svr.Post("/api/security/secrets", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            AgentConnectionSecret secret;
            secret.id = j.at("id");
            secret.type = j.at("type");
            secret.keyData = j.at("keyData");
            if (j.contains("groupName")) secret.groupName = j["groupName"];
            if (j.contains("isControlPlane")) secret.isControlPlane = j["isControlPlane"];
            m_model.addConnectionSecret(secret);
            res.set_content("Connection Secret Added", "text/plain");
        } catch (...) { res.status = 400; }
    });

    m_svr.Delete("/api/security/secrets", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            m_model.deleteConnectionSecret(j.at("id"));
            res.set_content("Connection Secret Deleted", "text/plain");
        } catch (...) { res.status = 400; }
    });

    m_svr.Post("/api/security/secrets/group", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string id = j.at("id");
            if (j.contains("groupName") && !j.at("groupName").get<std::string>().empty()) {
                m_model.groupConnectionSecret(id, j.at("groupName"));
            } else {
                m_model.ungroupConnectionSecret(id);
            }
            res.set_content("Connection Secret Group Updated", "text/plain");
        } catch (...) { res.status = 400; }
    });

    m_svr.Post("/api/connectors/resume", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            m_model.resumeConnector(j.at("id"));
            res.set_content("Connector Resumed", "text/plain");
        } catch (...) { res.status = 400; }
    });

    m_svr.Post("/api/connectors/terminate", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto j = json::parse(req.body);
            m_model.removeConnector(j.at("id"));
            res.set_content("Connector Terminated", "text/plain");
        } catch (...) { res.status = 400; }
    });
}

void AgenticHttpController::runPoller() { /* simplified */ }
void AgenticHttpController::executePoll(const AgentResource& res) { /* simplified */ }
