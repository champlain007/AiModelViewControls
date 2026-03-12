#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <iomanip>
#include <sstream>

enum class SchemeType {
    BUILTIN_AES,
    CUSTOM_SHELL
};

struct CryptoScheme {
    std::string id;
    SchemeType type;
    std::string encrypt_cmd;
    std::string decrypt_cmd;
};

class CryptoManager {
public:
    CryptoManager();
    ~CryptoManager();

    void registerScheme(const CryptoScheme& scheme);
    void setActiveScheme(const std::string& schemeID);
    std::string getActiveSchemeID() const;
    std::vector<std::string> getAvailableSchemes();

    std::string generateKey(); 
    bool rotateKey(); 
    std::string getCurrentKeyID() const;

    std::string encrypt(const std::string& plaintext);
    std::string decrypt(const std::string& ciphertext);

private:
    struct Key {
        std::string id;
        std::vector<unsigned char> data;
    };

    std::mutex m_mutex;
    std::map<std::string, Key> m_keys;
    std::string m_current_key_id;

    std::map<std::string, CryptoScheme> m_schemes;
    std::string m_active_scheme_id;

    std::string aesEncrypt(const std::string& plaintext);
    std::string aesDecrypt(const std::string& ciphertext);
    std::string shellExec(const std::string& cmd, const std::string& input);

    std::string toHex(const std::vector<unsigned char>& data);
    std::vector<unsigned char> fromHex(const std::string& hex);
};
