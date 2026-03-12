#include "docker/DockerFacade.hpp"
#include <httplib.h>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

DockerFacade::DockerFacade(const std::string& host, int port) : m_host(host), m_port(port) {
    m_cli = new httplib::Client(m_host, m_port);
}

DockerFacade::~DockerFacade() {
    delete m_cli;
}

void DockerFacade::printHelp() {
    std::cout << "AgenticMVCpipe Docker Command Emulation" << std::endl;
    std::cout << "Usage: ./AgenticPipeline --docker <command> [args...]" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  ps              List running agent instances (connectors)" << std::endl;
    std::cout << "  run <id> <type> <target> [sandbox_id]" << std::endl;
    std::cout << "                  Create and run a new agent instance" << std::endl;
    std::cout << "  stop <id>         Stop and remove an agent instance" << std::endl;
    std::cout << "  inspect <id>    Display detailed information on an instance" << std::endl;
}

int DockerFacade::execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        printHelp();
        return 1;
    }
    std::string cmd = args[0];
    std::vector<std::string> subArgs(args.begin() + 1, args.end());

    if (cmd == "ps") return cmdPs();
    if (cmd == "run") return cmdRun(subArgs);
    if (cmd == "stop") return cmdStop(subArgs);
    if (cmd == "inspect") return cmdInspect(subArgs);

    std::cerr << "Unknown Docker command: " << cmd << std::endl;
    printHelp();
    return 1;
}

int DockerFacade::cmdPs() {
    if (auto res = m_cli->Get("/api/hub")) {
        if (res->status == 200) {
            auto j = json::parse(res->body);
            std::cout << "INSTANCE ID\tTYPE\t\tTARGET\t\t\tSANDBOX" << std::endl;
            for (const auto& c : j) {
                // Convert type number to string for display
                int typeInt = c.value("type", -1);
                std::string typeStr = (typeInt == 1) ? "LOCAL_FILE" : "OTHER";

                std::cout << c.value("id", "N/A").substr(0, 12) << "\t"
                          << typeStr << "\t\t"
                          << c.value("target", "N/A") << "\t"
                          << c.value("sandbox_id", "default") << std::endl;
            }
            return 0;
        }
    }
    std::cerr << "Error: Could not connect to AgenticMVCpipe daemon on port " << m_port << "." << std::endl;
    return 1;
}

int DockerFacade::cmdRun(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cerr << "Usage: run <id> <type> <target> [sandbox_id]" << std::endl;
        return 1;
    }
    json j = {
        {"id", args[0]},
        {"type", std::stoi(args[1])},
        {"target", args[2]},
        {"sandbox_id", (args.size() > 3) ? args[3] : "default"}
    };
    if (auto res = m_cli->Post("/api/hub", j.dump(), "application/json")) {
        if (res->status == 200) {
            std::cout << "Successfully created instance: " << args[0] << std::endl;
            return 0;
        }
    }
    std::cerr << "Error: Failed to create instance." << std::endl;
    return 1;
}

int DockerFacade::cmdStop(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Usage: stop <id>" << std::endl;
        return 1;
    }
    json j = {{"id", args[0]}};
    if (auto res = m_cli->Delete("/api/hub", j.dump(), "application/json")) {
        if (res->status == 200) {
            std::cout << "Successfully stopped instance: " << args[0] << std::endl;
            return 0;
        }
    }
    std::cerr << "Error: Failed to stop instance." << std::endl;
    return 1;
}

int DockerFacade::cmdInspect(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Usage: inspect <id>" << std::endl;
        return 1;
    }
    if (auto res = m_cli->Get(("/api/hub/" + args[0]).c_str())) {
        if (res->status == 200) {
            auto j = json::parse(res->body);
            std::cout << j.dump(2) << std::endl;
            return 0;
        }
    }
    std::cerr << "Error: Could not inspect instance '" << args[0] << "'." << std::endl;
    return 1;
}
