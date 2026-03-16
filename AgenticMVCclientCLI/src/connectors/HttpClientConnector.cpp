#include "connectors/HttpClientConnector.hpp"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>

bool HttpClientConnector::connect(const std::string& target) {
    m_target = target;
    if (m_target.find("http") != 0) {
        m_target = "http://" + m_target;
    }
    std::cout << "[HTTP Connector] Connecting to: " << m_target << std::endl;
    return true;
}

std::string HttpClientConnector::sendPayload(const std::string& data) {
    httplib::Client cli(m_target);
    nlohmann::json j;
    j["payload"] = data;
    auto res = cli.Post("/api/send", j.dump(), "application/json");
    if (res) {
        if (res->status == 200) {
            return "[HTTP Response] " + res->body;
        } else {
            return "[HTTP Error] Status: " + std::to_string(res->status) + " Body: " + res->body;
        }
    } else {
        return "[HTTP Error] Failed to connect or send payload.";
    }
}

bool HttpClientConnector::sendAlert(const std::string& alertData) {
    httplib::Client cli(m_target);
    nlohmann::json j;
    j["alert"] = alertData;
    j["source"] = "AgenticMVCclientCLI";
    auto res = cli.Post("/api/alert", j.dump(), "application/json");
    return res && res->status == 200;
}

void HttpClientConnector::disconnect() {
    std::cout << "[HTTP Connector] Disconnected." << std::endl;
}
