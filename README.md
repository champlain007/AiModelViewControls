# AgenticMVCpipe

[![Security: Hardened](https://img.shields.io/badge/Security-Hardened-green.svg)](https://github.com/champlain007/AgenticMVCpipe)
[![Standard: C++20](https://img.shields.io/badge/Standard-C%2B%2B20-blue.svg)](https://github.com/champlain007/AgenticMVCpipe)

**AgenticMVCpipe** is a modular, decentralized, and security-hardened C++ framework for agentic orchestration, real-time distributed intelligence, and automated sandboxing.

## 🚀 Key Features

- **Architectural Excellence**: Fully adheres to modern design patterns (Factory, Strategy, State, Singleton, Builder, and Chain of Responsibility).
- **Security First**: 
  - **Real-Time Malware Scanning**: Integrated ingress/egress scanning using a modular scanner chain (ClamAV compatible).
  - **Data Leak Prevention (DLP)**: Transmissions are monitored for sensitive cryptographic keys and connection secrets.
  - **Instance Sandboxing**: Isolated execution environments with localized configuration and boot scripts.
- **Modular Ecosystem**:
  - **Core Node**: TUI-based Dashboard, Orchestrator, and Security Gateway.
  - **AgenticMVCserverCLI**: Lightweight, view-free core engine for headless server environments.
  - **AgenticMVCclientCLI**: Secure, view-free client for remote task submission.

## 🛠 Installation & Setup

### Requirements
- OS: Linux, macOS, or Windows (WSL)
- Compiler: C++17 or higher (C++20 recommended)
- Build Tool: CMake 3.10+
- Dependencies: OpenSSL, ZLIB, Threads (all handled via CMake)

### One-Click Quick Start
The system is autonomously managed via the `tradesecret.sh` script. This is the **only** authorized entry point for initialization and security handshaking.

```bash
git clone https://github.com/champlain007/AgenticMVCpipe.git
cd AgenticMVCpipe
chmod +x tradesecret.sh
./tradesecret.sh --interactive
```

## 📖 Command Manual

### `tradesecret.sh`
The primary bootstrapper and configuration tool.
- `-i, --interactive`: Launches a configuration prompt for ports and keys.
- (Default): Starts all core services and the Dashboard.

### Applications

| Application | Purpose |
| :--- | :--- |
| `AgenticPipeline` | Main node with Dashboard, Orchestrator, and MCP. |
| `AgenticMVCserverCLI` | Headless DockerEngine-like engine for automation. |
| `AgenticMVCclientCLI` | Secure task submission client with local DLP. |

## 🧩 Architectural Overview

- **Factory Pattern**: Dynamically initializes services (Orchestrator, MCP, CLI) based on startup flags.
- **State Pattern**: Manages node status (`NORMAL`, `WARNING`, `LOCKDOWN`) and enforces dynamic authorization.
- **Strategy Pattern**: Decouples connection types (HTTP, WebSocket, Local File) and security detectors.
- **Chain of Responsibility**: Sequences multiple malware scanners and data leak detectors for comprehensive coverage.

## 🛡 Security Governance
In the event of a malware detection or data leak:
1. The connection is **immediately suspended**.
2. The user is verbosely alerted via the TUI Dashboard or API.
3. The user has the choice to **Resume** (after review) or **Terminate** the connection.

## 📄 License
This project is for secure, decentralized agentic research and deployment. 

---
*Maintained by champlain007*
