#include "ui/CliInteractor.hpp"
#include <iostream>
#include <string>

CliInteractor::CliInteractor(AgenticApp& app, bool useHttp) : m_app(app), m_useHttp(useHttp) {}

void CliInteractor::run() {
    std::string line;
    std::cout << "AgenticMVCclientCLI (" << (m_useHttp ? "HTTP" : "Mock") << ")> " << std::flush;
    while (std::getline(std::cin, line)) {
        if (line == "exit" || line == "quit") break;
        if (line.find("send ") == 0) {
            std::string payload = line.substr(5);
            std::cout << m_app.executeTask(payload) << std::endl;
        } else if (line == "help") {
            std::cout << "Commands:\n"
                      << "  send <payload> - Scan and transmit data\n"
                      << "  exit, quit     - Exit the client\n"
                      << "  help           - Show this message\n";
        } else {
            std::cout << "Unknown command. Try: help, send <payload>, exit" << std::endl;
        }
        std::cout << "AgenticMVCclientCLI (" << (m_useHttp ? "HTTP" : "Mock") << ")> " << std::flush;
    }
}
