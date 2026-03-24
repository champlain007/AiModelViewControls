#include "cli/CliClient.hpp"
#include <iostream>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iomanip>

using json = nlohmann::json;

AgenticCliClient::AgenticCliClient(const std::string& host, int port) : m_host(host), m_port(port) {
    m_wayland.start();
}

AgenticCliClient::~AgenticCliClient() {
    m_wayland.stop();
}

void AgenticCliClient::printHelp() {
    std::cout << "AgenticMVCpipe CLI - Command Line Interface" << std::endl;
    std::cout << "Usage: ./AgenticPipeline --cli <command> [args...]" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  status    Show local node status" << std::endl;
    std::cout << "  alerts    Show recent security alerts" << std::endl;
    std::cout << "  ledger    Show private blockchain ledger" << std::endl;
    std::cout << "  nodes     Show orchestrator nodes (requires port 9000)" << std::endl;
    std::cout << "  sandbox   Manage Agent Sandboxes" << std::endl;
    std::cout << "    sandbox list" << std::endl;
    std::cout << "    sandbox create <id> <level:0-3>" << std::endl;
    std::cout << "    sandbox delete <id>" << std::endl;
    std::cout << "    sandbox revert <id> <version>" << std::endl;
}

int AgenticCliClient::executeCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        printHelp();
        return 1;
    }

    std::string cmd = args[0];
    if (cmd == "status") return cmdStatus();
    if (cmd == "alerts") return cmdAlerts();
    if (cmd == "ledger") return cmdLedger();
    if (cmd == "nodes") return cmdNodes();
    if (cmd == "sandbox") {
        std::vector<std::string> subArgs(args.begin() + 1, args.end());
        return cmdSandbox(subArgs);
    }

    std::cerr << "Unknown command: " << cmd << std::endl;
    printHelp();
    return 1;
}

int AgenticCliClient::cmdStatus() {
    httplib::Client cli(m_host, m_port);
    if (auto res = cli.Get("/api/state")) {
        if (res->status == 200) {
            auto j = json::parse(res->body);
            std::cout << "=== NODE STATUS ===" << std::endl;
            std::cout << "State:    " << j.value("state", "UNKNOWN") << std::endl;
            std::cout << "Isolated: " << (j.value("isolated", false) ? "YES" : "NO") << std::endl;
            return 0;
        }
    }
    std::cerr << "Error: Could not connect to local node on port " << m_port << std::endl;
    return 1;
}

int AgenticCliClient::cmdAlerts() {
    httplib::Client cli(m_host, m_port);
    if (auto res = cli.Get("/api/state")) {
        if (res->status == 200) {
            auto j = json::parse(res->body);
            std::cout << "=== RECENT ALERTS ===" << std::endl;
            if (j.contains("alerts") && j["alerts"].is_array()) {
                for (const auto& a : j["alerts"]) {
                    std::cout << "- [" << a.value("type", "") << "] Agent: " << a.value("agent", "") 
                              << " | Details: " << a.value("details", "") << std::endl;
                }
            }
            return 0;
        }
    }
    std::cerr << "Error: Could not connect to local node on port " << m_port << std::endl;
    return 1;
}

int AgenticCliClient::cmdLedger() {
    httplib::Client cli(m_host, m_port);
    if (auto res = cli.Get("/api/ledger")) {
        if (res->status == 200) {
            auto j = json::parse(res->body);
            std::cout << "=== PRIVATE BLOCKCHAIN LEDGER ===" << std::endl;
            for (const auto& b : j) {
                std::cout << "Block #" << b.value("index", 0) << " | Hash: " << b.value("hash", "") << std::endl;
            }
            return 0;
        }
    }
    std::cerr << "Error: Could not fetch ledger from local node." << std::endl;
    return 1;
}

int AgenticCliClient::cmdNodes() {
    httplib::Client cli(m_host, 9000); // Default Orchestrator Port
    if (auto res = cli.Get("/api/orchestrator/nodes")) {
        if (res->status == 200) {
            auto j = json::parse(res->body);
            std::cout << "=== ORCHESTRATOR NODES ===" << std::endl;
            for (const auto& n : j) {
                std::cout << "- Node: " << n.value("name", "Unknown") 
                          << " | URL: " << n.value("url", "") 
                          << " | Status: " << n.value("status", "") << std::endl;
            }
            return 0;
        }
    }
    std::cerr << "Error: Could not connect to orchestrator on port 9000." << std::endl;
    return 1;
}

int AgenticCliClient::cmdSandbox(const std::vector<std::string>& args) {
    if (args.empty()) {
        printHelp();
        return 1;
    }
    
    std::string subCmd = args[0];
    httplib::Client cli(m_host, m_port);
    
    if (subCmd == "list") {
        if (auto res = cli.Get("/api/sandbox")) {
            if (res->status == 200) {
                auto j = json::parse(res->body);
                std::cout << "=== AGENT SANDBOXES ===" << std::endl;
                for (const auto& p : j) {
                    std::cout << "- ID: " << p.value("id", "") 
                              << " | v" << p.value("version", 1) 
                              << " | Level: " << p.value("level", 0) << std::endl;
                }
                return 0;
            }
        }
        std::cerr << "Failed to fetch sandboxes." << std::endl;
        return 1;
    } 
    else if (subCmd == "create") {
        if (args.size() < 3) { std::cerr << "Usage: sandbox create <id> <level:0-3>" << std::endl; return 1; }
        json j = {
            {"id", args[1]},
            {"level", std::stoi(args[2])}
        };
        if (auto res = cli.Post("/api/sandbox", j.dump(), "application/json")) {
            if (res->status == 200) {
                std::cout << res->body << std::endl;
                return 0;
            }
        }
        std::cerr << "Failed to create sandbox." << std::endl;
        return 1;
    }
    else if (subCmd == "delete") {
        if (args.size() < 2) { std::cerr << "Usage: sandbox delete <id>" << std::endl; return 1; }
        json j = {{"id", args[1]}};
        if (auto res = cli.Delete("/api/sandbox", j.dump(), "application/json")) {
            if (res->status == 200) {
                std::cout << res->body << std::endl;
                return 0;
            }
        }
        std::cerr << "Failed to delete sandbox." << std::endl;
        return 1;
    }
    else if (subCmd == "revert") {
        if (args.size() < 3) { std::cerr << "Usage: sandbox revert <id> <version>" << std::endl; return 1; }
        json j = {{"id", args[1]}, {"version", std::stoi(args[2])}};
        if (auto res = cli.Post("/api/sandbox/revert", j.dump(), "application/json")) {
            if (res->status == 200) {
                std::cout << res->body << std::endl;
                return 0;
            }
        }
        std::cerr << "Failed to revert sandbox." << std::endl;
        return 1;
    }
    
    std::cerr << "Unknown sandbox command: " << subCmd << std::endl;
    return 1;
}