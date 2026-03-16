#include "AgenticApp.hpp"
#include "core/SessionManager.hpp"
#include "core/AlertDispatcher.hpp"
#include "security/MalwareScanner.hpp"
#include "security/DlpScanner.hpp"
#include "security/PromptInjectionScanner.hpp"
#include "security/SocialEngineeringScanner.hpp"
#include "security/CognitiveHackingScanner.hpp"
#include "security/ObfuscationScanner.hpp"
#include "security/IndirectInjectionScanner.hpp"
#include "formatters/UserMarkdownFormatter.hpp"
#include "formatters/SpotlightFormatter.hpp"
#include "connectors/HttpClientConnector.hpp"
#include "connectors/MockClientConnector.hpp"
#include "ui/CliInteractor.hpp"
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char** argv) {
    std::string target = "localhost:8080";
    std::string avCmd = "clamscan --no-summary %f";
    bool useHttp = false;
    bool syncAlerts = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--http") {
            useHttp = true;
        } else if (arg == "--alert-sync") {
            syncAlerts = true;
        } else if (arg.find("--target=") == 0) {
            target = arg.substr(9);
        } else if (arg.find("--av-cmd=") == 0) {
            avCmd = arg.substr(9);
        } else if (arg[0] != '-') {
            target = arg;
        }
    }

    // Dependency Injection
    std::unique_ptr<IClientConnector> connector;
    if (useHttp) {
        connector = std::make_unique<HttpClientConnector>();
    } else {
        connector = std::make_unique<MockClientConnector>();
    }
    
    IClientConnector* connectorPtr = connector.get();

    auto sessionState = std::make_unique<SessionManager>();
    auto alertHandler = std::make_unique<AlertDispatcher>(connectorPtr, syncAlerts);
    
    auto securityPipeline = std::make_unique<SecurityPipeline>(alertHandler.get());
    securityPipeline->addScanner(std::make_unique<MalwareScanner>(avCmd));
    securityPipeline->addScanner(std::make_unique<DlpScanner>());
    securityPipeline->addScanner(std::make_unique<PromptInjectionScanner>());
    securityPipeline->addScanner(std::make_unique<SocialEngineeringScanner>());
    securityPipeline->addScanner(std::make_unique<CognitiveHackingScanner>());
    securityPipeline->addScanner(std::make_unique<ObfuscationScanner>());
    securityPipeline->addScanner(std::make_unique<IndirectInjectionScanner>());

    auto formatterPipeline = std::make_unique<FormatterPipeline>();
    formatterPipeline->addFormatter(std::make_unique<UserMarkdownFormatter>());
    formatterPipeline->addFormatter(std::make_unique<SpotlightFormatter>());

    AgenticApp app(std::move(connector), std::move(sessionState), std::move(alertHandler),
                   std::move(securityPipeline), std::move(formatterPipeline));

    if (!app.initialize(target)) {
        std::cerr << "Failed to initialize AgenticApp." << std::endl;
        return 1;
    }

    CliInteractor ui(app, useHttp);
    ui.run();

    app.shutdown();
    return 0;
}
