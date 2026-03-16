# AiModelViewControls (AiMVCs)

## 🛡️ Enterprise-Grade Agentic AI Orchestration Framework

AiModelViewControls (AiMVCs) is a comprehensive C++ framework designed for the secure management, orchestration, and pipelining of distributed Agentic AI nodes. It provides a robust Model-View-Controller (MVC) architecture focused on sandboxed execution, real-time threat detection, and seamless node-to-node handshaking.

---

## 🏗️ Core Architecture

### AiModelViewControls (Facade)
The primary entry point of the system. It abstracts the underlying complexities of the orchestrator and service factories, providing a high-level API for framework initialization.

### AgenticMVCpipe (Pipeline Engine)
A specialized sub-component of AiMVCs responsible for:
- Creating secure connection chains between distributed nodes.
- Managing handshakes and authentication pipelines.
- Ensuring data integrity across the transmission stream.

---

## 🛠️ Sub-Projects

### 1. AgenticMVCclientCLI (Secure Client)
A highly modular, SRP-compliant C++ CLI client. It acts as the "Secure Edge" of the framework, performing mandatory pre-flight security scans including:
- **Malware Detection:** Generic AV integration (configurable to use ClamAV or any third-party scanner).
- **DLP (Data Loss Prevention):** Scans for leaked RSA keys, AWS credentials, and PII.
- **Red Team Heuristics:** Native detection of Jailbreaks, Crescendo attacks, Obfuscation (Base64/Spacing), and Cognitive Hacking.
- **Alert Syncing:** Real-time push of locally caught threats to the Parent Orchestrator.

### 2. AgenticMVCserverCLI (Agent Engine)
A lightweight, headless C++ engine designed for task execution within isolated sandboxes. It provides the execution environment for the model's instructions while maintaining strict local guardrails.

---

## 🚀 Quick Start

### Prerequisites
- **CMake** (3.25+)
- **C++20/26** Compiler (Clang/GCC)
- **OpenSSL** & **ZLIB**
- **ClamAV** (Optional, for local malware scanning)

### Initialization & Startup
The framework is strictly managed via the `tradesecret.sh` bootstrapper. This script handles all environment configurations, port assignments, and handshake keys.

```bash
# 1. Clone the repository
git clone https://github.com/champlain007/AiModelViewControls.git
cd AiModelViewControls

# 2. Launch the framework
chmod +x tradesecret.sh
./tradesecret.sh --interactive
```

---

## 📄 Documentation
- **[Manual Pages](./manual/COMMANDS.md):** Full command reference for all framework components.
- **[Troubleshooting Guide](./help/TROUBLESHOOTING.md):** Detailed help with handshaking and security violations.

---

## ⚖️ License & Security
AiModelViewControls is built with a security-first mindset. All egress data from clients is spotlighted (Defensive Encoding) to prevent prompt injection and ensure downstream models prioritize system instructions over untrusted user data.
