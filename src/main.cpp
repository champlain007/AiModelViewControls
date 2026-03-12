#include "AgenticServiceFactory.hpp"
#include "AgenticConfigManager.hpp"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    try {
        std::string instanceId = "";
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--instance" && i + 1 < argc) {
                instanceId = argv[i + 1];
            }
        }

        auto& config = AgenticConfigManager::getInstance();
        config.initializeSandbox(instanceId);

        if (AgenticServiceFactory::requiresTradeSecret(argc, argv)) {
            config.ensureLocalizedTradeSecret();
            
            const char* handshake = std::getenv("TRADESECRET_HANDSHAKE_KEY");
            if (!handshake || std::string(handshake) != "APPROVED_BY_TRADESECRET") {
                std::cerr << "[SECURITY_FATAL] AgenticMVCpipe must be exclusively configured and booted via the 'tradesecret.sh' script." << std::endl;
                return 1;
            }
        }

        auto service = AgenticServiceFactory::createService(argc, argv);
        if (service) {
            return service->run();
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[AGENT_SYSTEM] CRITICAL FATAL EXCEPTION: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "[AGENT_SYSTEM] CRITICAL UNKNOWN FATAL EXCEPTION" << std::endl;
        return 1;
    }
}
