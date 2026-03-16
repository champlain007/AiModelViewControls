#include "connectors/MockClientConnector.hpp"
#include <iostream>

bool MockClientConnector::connect(const std::string& target) {
    std::cout << "[Connector] (Mock) Connected to target: " << target << std::endl;
    return true;
}

std::string MockClientConnector::sendPayload(const std::string& data) {
    return "SERVER_RESPONSE (Mock): Received " + std::to_string(data.size()) + " bytes.";
}

bool MockClientConnector::sendAlert(const std::string& alertData) {
    std::cout << "[Connector] (Mock) Alert Synced: " << alertData << std::endl;
    return true;
}

void MockClientConnector::disconnect() {
    std::cout << "[Connector] (Mock) Disconnected." << std::endl;
}
