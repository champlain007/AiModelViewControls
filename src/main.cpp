#include "AiModelViewControls.hpp"
#include "AgenticServiceFactory.hpp"
#include "AgenticConfigManager.hpp"
#include <iostream>
#include <string>

/**
 * @brief Entry point for the AiModelViewControls system.
 */
int main(int argc, char** argv) {
    try {
        std::string instanceId = "";
        int targetPort = 9000;

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--instance" && i + 1 < argc) {
                instanceId = argv[i + 1];
            }
        }

        // Initialize Facade
        AiModelViewControls framework;
        std::cout << "[System] Starting AiModelViewControls framework..." << std::endl;

        auto& config = AgenticConfigManager::getInstance();
        config.initializeSandbox(instanceId);

        if (AgenticServiceFactory::requiresTradeSecret(argc, argv)) {
            config.ensureLocalizedTradeSecret();
            
            const char* handshake = std::getenv("TRADESECRET_HANDSHAKE_KEY");
            if (!handshake || std::string(handshake) != "APPROVED_BY_TRADESECRET") {
                std::cerr << "[SECURITY_FATAL] AiModelViewControls nodes must be exclusively configured and booted via the 'tradesecret.sh' script." << std::endl;
                return 1;
            }
        }

        // Use the pipeline sub-class to manage connections
        framework.initialize(targetPort);
        framework.getPipeline()->createPipeline({"localhost:8080", "localhost:8081"});

        auto service = AgenticServiceFactory::createService(argc, argv);
        if (service) {
            return service->run();
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[AiMVCs] CRITICAL FATAL EXCEPTION: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "[AiMVCs] CRITICAL UNKNOWN FATAL EXCEPTION" << std::endl;
        return 1;
    }
}
