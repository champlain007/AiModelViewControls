#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "Crypto.hpp"

using json = nlohmann::json;

struct AgentLedgerBlock {
    int index;
    long long timestamp;
    json data; // Contains issued tokens or vaulted secrets
    std::string prevHash;
    std::string hash;

    std::string calculateHash() const;
    json toJson() const;
};

class AgentLedger {
public:
    AgentLedger();
    void addBlock(const json& data);
    bool isValid() const;
    bool isValidBlock(const AgentLedgerBlock& newBlock, const AgentLedgerBlock& prevBlock) const;
    void mergeChain(const std::vector<AgentLedgerBlock>& externalChain);
    void saveToFile(const std::string& path, CryptoManager* crypto = nullptr);
    void loadFromFile(const std::string& path, CryptoManager* crypto = nullptr);
    std::vector<AgentLedgerBlock> getChain() const { return m_chain; }
    AgentLedgerBlock getLatestBlock() const { return m_chain.back(); }

private:
    std::vector<AgentLedgerBlock> m_chain;
    std::string calculateBlockHash(const AgentLedgerBlock& b) const;
};
