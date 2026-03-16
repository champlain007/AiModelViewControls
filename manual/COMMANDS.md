# MANUAL: AiModelViewControls Commands

## tradesecret.sh
**Usage:** `./tradesecret.sh [-i | --interactive]`

The primary configuration and boot script for the AiModelViewControls (AiMVCs) framework. It manages environment secrets and service orchestration.

## AiModelViewControls
**Usage:** `./AiModelViewControls [OPTIONS]`

The core framework executable.
- `--instance <id>`: Launches a specific sandboxed instance.
- `--orchestrator`: Starts the central supervisor service.
- `--mcp`: Launches the Model Control Protocol gateway.
- `--ws-sandbox`: Starts the isolated WebSocket environment.

## AgenticMVCclientCLI
**Usage:** `./AgenticMVCclientCLI [OPTIONS]`

Secure client for interacting with nodes.
- `--http`: Use real HTTP transmission instead of the mock connector.
- `--target=<url>`: Specify the target engine (default: localhost:8080).
- `--alert-sync`: Enable real-time alert synchronization to the central orchestrator.
- `--av-cmd="<command>"`: Configure a custom anti-virus/malware scanner command. Use `%f` as a placeholder for the file path (e.g., `--av-cmd="clamscan --no-summary %f"`).

**Interactive Commands:**
- `send <payload>`: Scans (Malware/DLP/RedTeam) and transmits data.
- `help`: Displays local command help.
- `exit`: Shuts down the client.

## AgenticMVCserverCLI
**Usage:** `./AgenticMVCserverCLI`

Local engine for executing sandboxed AI tasks.
