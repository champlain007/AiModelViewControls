#include "Crypto.hpp"
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <vector>

// Hardcoded node master key for demonstration (in production, loaded from secure HSM)
static const unsigned char MASTER_KEY[32] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
};

CryptoManager::CryptoManager() {
    m_active_scheme_id = "aes-256-cbc";
    m_current_key_id = "master-node-key-v1";
}

CryptoManager::~CryptoManager() {}

void CryptoManager::registerScheme(const CryptoScheme& scheme) {}
void CryptoManager::setActiveScheme(const std::string& schemeID) {}
std::string CryptoManager::getActiveSchemeID() const { return m_active_scheme_id; }
std::vector<std::string> CryptoManager::getAvailableSchemes() { return {m_active_scheme_id}; }
std::string CryptoManager::generateKey() { return m_current_key_id; }
bool CryptoManager::rotateKey() { return true; }
std::string CryptoManager::getCurrentKeyID() const { return m_current_key_id; }

std::string CryptoManager::encrypt(const std::string& plaintext) { 
    return aesEncrypt(plaintext); 
}

std::string CryptoManager::decrypt(const std::string& ciphertext) { 
    return aesDecrypt(ciphertext); 
}

std::string CryptoManager::shellExec(const std::string& cmd, const std::string& input) { return input; }

std::string CryptoManager::aesEncrypt(const std::string& plaintext) {
    if (plaintext.empty()) return "";
    
    unsigned char iv[16];
    if (RAND_bytes(iv, sizeof(iv)) != 1) return "";

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return "";

    std::vector<unsigned char> ciphertext(plaintext.length() + EVP_MAX_BLOCK_LENGTH);
    int len = 0, ciphertext_len = 0;

    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, MASTER_KEY, iv);
    EVP_EncryptUpdate(ctx, ciphertext.data(), &len, (const unsigned char*)plaintext.c_str(), plaintext.length());
    ciphertext_len = len;

    EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len);
    ciphertext_len += len;
    
    EVP_CIPHER_CTX_free(ctx);

    std::vector<unsigned char> finalData;
    finalData.insert(finalData.end(), iv, iv + 16);
    finalData.insert(finalData.end(), ciphertext.begin(), ciphertext.begin() + ciphertext_len);

    return toHex(finalData);
}

std::string CryptoManager::aesDecrypt(const std::string& hexCiphertext) {
    if (hexCiphertext.empty()) return "";
    
    std::vector<unsigned char> rawData = fromHex(hexCiphertext);
    if (rawData.size() < 16) return "";

    unsigned char iv[16];
    std::copy(rawData.begin(), rawData.begin() + 16, iv);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return "";

    std::vector<unsigned char> plaintext(rawData.size());
    int len = 0, plaintext_len = 0;

    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, MASTER_KEY, iv);
    EVP_DecryptUpdate(ctx, plaintext.data(), &len, rawData.data() + 16, rawData.size() - 16);
    plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return ""; // Decryption failed
    }
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);

    return std::string((char*)plaintext.data(), plaintext_len);
}

std::string CryptoManager::toHex(const std::vector<unsigned char>& data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char c : data) ss << std::setw(2) << (int)c;
    return ss.str();
}

std::vector<unsigned char> CryptoManager::fromHex(const std::string& hex) {
    std::vector<unsigned char> data;
    for (size_t i = 0; i < hex.length(); i += 2) {
        data.push_back((unsigned char)strtol(hex.substr(i, 2).c_str(), NULL, 16));
    }
    return data;
}
