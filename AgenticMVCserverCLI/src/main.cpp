#include "IAgenticCore.hpp"
#include <iostream>
#include <atomic>
#include <map>
#include <mutex>
#include <thread>
#include <filesystem>
#include <fstream>

/**
 * @brief Lightweight implementation of the Agentic Engine.
 * Similar to DockerEngine, it runs headless and manages local resources.
 */
class AgenticMVCserverCLI : public IAgenticCore {
public:
    AgenticMVCserverCLI() : m_running(false) {}

    bool initialize() override {
        std::cout << "[Engine] Initializing AgenticMVCserverCLI Core..." << std::endl;
        // Check for security prerequisites (AV, etc.)
        if (std::system("clamscan --version > /dev/null 2>&1") != 0) {
            std::cerr << "[Security] WARNING: ClamAV not found. Engine will alert on every untrusted stream." << std::endl;
        }
        return true;
    }

    void start() override {
        m_running = true;
        std::cout << "[Engine] Agentic Engine Started (View-Free CLI mode)." << std::endl;
    }

    void stop() override {
        m_running = false;
        std::cout << "[Engine] Agentic Engine Stopped." << std::endl;
    }

    bool isRunning() const override { return m_running; }

    std::string executeTask(const std::string& taskPayload) override {
        if (!m_running) return "ERROR: Engine not running";

        // --- MANDATORY SECURITY SCAN (Strategy-based) ---
        if (!performSecurityScan(taskPayload)) {
            return "SECURITY_VIOLATION: Payload blocked by Malware/DLP scanner.";
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        std::string taskId = "task_" + std::to_string(m_tasks.size() + 1);
        m_tasks[taskId] = "COMPLETED";
        
        std::cout << "[Engine] Executed Task: " << taskId << std::endl;
        return "SUCCESS: " + taskId;
    }

    std::string getStatus() const override {
        return m_running ? "RUNNING" : "STOPPED";
    }

private:
    bool performSecurityScan(const std::string& data) {
        // Lightweight shell-out to AV or internal DLP patterns
        // This works with any generic scanner available in the OS path
        std::string tempFile = "/tmp/agent_cli_scan.tmp";
        std::ofstream ofs(tempFile);
        ofs << data;
        ofs.close();

        // Check for malware (Example: ClamAV)
        int res = std::system(("clamscan --no-summary " + tempFile + " > /dev/null 2>&1").c_str());
        std::filesystem::remove(tempFile);

        if (res == 1) {
            std::cerr << "[ALERT] MALWARE DETECTED in CLI stream. Connection Suspending." << std::endl;
            return false;
        }

        // Generic Key Leak Detection (Lightweight version)
        if (data.find("BEGIN RSA PRIVATE KEY") != std::string::npos) {
            std::cerr << "[ALERT] DLP: Potential Key Leak detected. Suspending." << std::endl;
            return false;
        }

        return true;
    }

    std::atomic<bool> m_running;
    std::map<std::string, std::string> m_tasks;
    mutable std::mutex m_mutex;
};

int main(int argc, char** argv) {
    auto engine = std::make_unique<AgenticMVCserverCLI>();
    
    if (!engine->initialize()) return 1;
    engine->start();

    // Simple command loop for the view-free CLI
    std::string line;
    std::cout << "AgenticMVCserverCLI> " << std::flush;
    while (engine->isRunning() && std::getline(std::cin, line)) {
        if (line == "exit" || line == "quit") break;
        if (line == "status") {
            std::cout << "Status: " << engine->getStatus() << std::endl;
        } else if (line.find("exec ") == 0) {
            std::string payload = line.substr(5);
            std::cout << engine->executeTask(payload) << std::endl;
        } else {
            std::cout << "Unknown command. Try: status, exec <payload>, exit" << std::endl;
        }
        std::cout << "AgenticMVCserverCLI> " << std::flush;
    }

    engine->stop();
    return 0;
}
