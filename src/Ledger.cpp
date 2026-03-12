#include "Ledger.hpp"
#include <sstream>
#include <fstream>
#include <iostream>
#include <chrono>
#include <filesystem>

std::string AgentLedgerBlock::calculateHash() const {
    std::stringstream ss;
    ss << index << timestamp << data.dump() << prevHash;
    // Simple hash for prototype (In production use SHA-256 via CryptoManager)
    size_t h = std::hash<std::string>{}(ss.str());
    return std::to_string(h);
}

json AgentLedgerBlock::toJson() const {
    return json{{"index", index}, {"timestamp", timestamp}, {"data", data}, {"prev_hash", prevHash}, {"hash", hash}};
}

AgentLedger::AgentLedger() {
    // Create Genesis Block for the Agentic Ledger
    AgentLedgerBlock genesis;
    genesis.index = 0;
    genesis.timestamp = 0;
    genesis.data = "Genesis Block - AgenticMVCpipe Ledger";
    genesis.prevHash = "0";
    genesis.hash = genesis.calculateHash();
    m_chain.push_back(genesis);
}

void AgentLedger::addBlock(const json& data) {
    AgentLedgerBlock last = m_chain.back();
    AgentLedgerBlock next;
    next.index = last.index + 1;
    next.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    next.data = data;
    next.prevHash = last.hash;
    next.hash = next.calculateHash();
    m_chain.push_back(next);
}

bool AgentLedger::isValid() const {
    for (size_t i = 1; i < m_chain.size(); i++) {
        const AgentLedgerBlock& current = m_chain[i];
        const AgentLedgerBlock& prev = m_chain[i-1];
        if (current.hash != current.calculateHash()) return false;
        if (current.prevHash != prev.hash) return false;
    }
    return true;
}

bool AgentLedger::isValidBlock(const AgentLedgerBlock& newBlock, const AgentLedgerBlock& prevBlock) const {
    if (newBlock.index != prevBlock.index + 1) return false;
    if (newBlock.prevHash != prevBlock.hash) return false;
    if (newBlock.hash != newBlock.calculateHash()) return false;
    return true;
}

void AgentLedger::mergeChain(const std::vector<AgentLedgerBlock>& externalChain) {
    if (externalChain.empty()) return;

    // Simple consensus: if the external chain is longer and valid, we adopt it
    // In a more complex Agentic system, we would merge based on timestamps/proofs
    if (externalChain.size() > m_chain.size()) {
        // Validate external chain integrity before adoption
        bool valid = true;
        for (size_t i = 1; i < externalChain.size(); i++) {
            if (!isValidBlock(externalChain[i], externalChain[i-1])) {
                valid = false; break;
            }
        }

        if (valid) {
            std::cout << "[AGENT_LEDGER] Syncing with external chain (New Size: " << externalChain.size() << ")" << std::endl;
            m_chain = externalChain;
        }
    }
}

void AgentLedger::saveToFile(const std::string& path, CryptoManager* crypto) {
    json j = json::array();
    for (const auto& b : m_chain) j.push_back(b.toJson());

    std::string content = j.dump(2);

    if (crypto) {
        content = crypto->encrypt(content);
    }

    std::ofstream f(path);
    if (f.is_open()) {
        f << content;
        f.close();

        // Enforce strict file permissions for maximum privacy natively in a cross-platform way
        try {
            std::filesystem::permissions(path, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write, std::filesystem::perm_options::replace);
        } catch (...) {}
    }
}

void AgentLedger::loadFromFile(const std::string& path, CryptoManager* crypto) {
    std::ifstream f(path);
    if (!f.is_open()) return;

    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string content = buffer.str();

    if (content.empty()) return;

    try {
        json j;
        // Check if content is plaintext JSON (starts with '[' or '{')
        if (content[0] == '[' || content[0] == '{') {
            j = json::parse(content);
        } else if (crypto) {
            // It must be encrypted hex, try to decrypt
            std::string decrypted = crypto->decrypt(content);
            if (!decrypted.empty()) {
                j = json::parse(decrypted);
            }
        }

        if (j.is_array()) {
            m_chain.clear();
            for (const auto& bJson : j) {
                AgentLedgerBlock b;
                b.index = bJson.value("index", 0);
                b.timestamp = bJson.value("timestamp", 0LL);
                b.data = bJson.value("data", json({}));
                b.prevHash = bJson.value("prev_hash", "");
                b.hash = bJson.value("hash", "");
                m_chain.push_back(b);
            }
        }
    } catch (...) {}
}
