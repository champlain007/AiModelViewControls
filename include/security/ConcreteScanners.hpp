#pragma once
#include "IMalwareScanner.hpp"
#include <cstdlib>
#include <fstream>
#include <filesystem>

class ClamAvScanner : public IMalwareScanner {
public:
    std::string getName() const override { return "ClamAV"; }
    MalwareInfo scan(const std::string& data) override {
        // Check if clamscan is available
        if (std::system("clamscan --version > /dev/null 2>&1") != 0) {
            return { ScanResult::NOT_FOUND, "clamscan binary not found in path", getName() };
        }

        std::string tempFile = "scan_tmp_" + std::to_string(std::time(nullptr)) + ".dat";
        std::ofstream ofs(tempFile, std::ios::binary);
        ofs.write(data.data(), data.size());
        ofs.close();

        int res = std::system(("clamscan --no-summary " + tempFile + " > /dev/null 2>&1").c_str());
        std::filesystem::remove(tempFile);

        if (res == 0) return { ScanResult::CLEAN, "No viruses found.", getName() };
        if (res == 1) return { ScanResult::INFECTED, "Malware detected by ClamAV!", getName() };
        return { ScanResult::ERROR, "ClamAV returned an error code.", getName() };
    }
};

class UserConfigScanner : public IMalwareScanner {
public:
    UserConfigScanner(const std::string& cmd) : m_cmd(cmd) {}
    std::string getName() const override { return "UserConfiguredAV"; }
    MalwareInfo scan(const std::string& data) override {
        if (m_cmd.empty()) return { ScanResult::NOT_FOUND, "No custom command configured", getName() };
        
        // Simulation of a custom scanner integration
        if (m_cmd.find("block_all") != std::string::npos) {
             return { ScanResult::INFECTED, "Custom AV blocked this payload.", getName() };
        }
        
        return { ScanResult::CLEAN, "Custom AV passed.", getName() };
    }
private:
    std::string m_cmd;
};
