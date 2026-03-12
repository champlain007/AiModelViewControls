#include "IAgenticClient.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <memory>

class AgenticMVCclientCLI {
public:
    AgenticMVCclientCLI(std::unique_ptr<IClientConnector> connector) 
        : m_connector(std::move(connector)) {}

    bool initialize(const std::string& target) {
        std::cout << "[Client] Initializing AgenticMVCclientCLI..." << std::endl;
        if (std::system("clamscan --version > /dev/null 2>&1") != 0) {
            std::cerr << "[Security] WARNING: Local AV (clamscan) not found. Client will alert on untrusted egress streams." << std::endl;
        }
        return m_connector->connect(target);
    }

    std::string executeTask(const std::string& payload) {
        // --- LOCAL MANDATORY SECURITY SCAN (Strategy-based) ---
        if (!performSecurityScan(payload)) {
            return "SECURITY_VIOLATION: Payload blocked by local Malware/DLP scanner before transmission.";
        }

        std::cout << "[Client] Security checks passed. Transmitting payload..." << std::endl;
        return m_connector->sendPayload(payload);
    }

    void shutdown() {
        m_connector->disconnect();
        std::cout << "[Client] Agentic Client disconnected and shut down." << std::endl;
    }

private:
    bool performSecurityScan(const std::string& data) {
        std::string tempFile = "/tmp/agent_client_scan.tmp";
        std::ofstream ofs(tempFile);
        ofs << data;
        ofs.close();

        // 1. Malware Scan
        int res = std::system(("clamscan --no-summary " + tempFile + " > /dev/null 2>&1").c_str());
        std::filesystem::remove(tempFile);

        if (res == 1) {
            std::cerr << "[ALERT] MALWARE DETECTED in egress payload. Transmission blocked." << std::endl;
            return false;
        }

        // 2. DLP Key Leak Detection
        if (data.find("BEGIN RSA PRIVATE KEY") != std::string::npos ||
            data.find("TRADESECRET_HANDSHAKE_KEY") != std::string::npos) {
            std::cerr << "[ALERT] DLP: Potential Key/Secret Leak detected. Transmission blocked." << std::endl;
            return false;
        }

        return true;
    }

    std::unique_ptr<IClientConnector> m_connector;
};

// Mock concrete strategy for demonstration (could be HTTP or IPC in a real scenario)
class MockClientConnector : public IClientConnector {
public:
    bool connect(const std::string& target) override {
        std::cout << "[Connector] Connected to target: " << target << std::endl;
        return true;
    }
    
    std::string sendPayload(const std::string& data) override {
        return "SERVER_RESPONSE: Received " + std::to_string(data.size()) + " bytes.";
    }
    
    void disconnect() override {
        std::cout << "[Connector] Disconnected." << std::endl;
    }
};

int main(int argc, char** argv) {
    std::string target = "localhost:8080";
    if (argc > 1) {
        target = argv[1];
    }

    auto connector = std::make_unique<MockClientConnector>();
    AgenticMVCclientCLI client(std::move(connector));

    if (!client.initialize(target)) {
        std::cerr << "Failed to connect to target." << std::endl;
        return 1;
    }

    std::string line;
    std::cout << "AgenticMVCclientCLI> " << std::flush;
    while (std::getline(std::cin, line)) {
        if (line == "exit" || line == "quit") break;
        if (line.find("send ") == 0) {
            std::string payload = line.substr(5);
            std::cout << client.executeTask(payload) << std::endl;
        } else {
            std::cout << "Unknown command. Try: send <payload>, exit" << std::endl;
        }
        std::cout << "AgenticMVCclientCLI> " << std::flush;
    }

    client.shutdown();
    return 0;
}