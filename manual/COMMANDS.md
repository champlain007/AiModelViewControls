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
- `--http`: Real-world network mode.
- `--alert-sync`: Pushes local Red Team alerts to the parent orchestrator.

## AgenticMVCserverCLI
**Usage:** `./AgenticMVCserverCLI`

Local engine for executing sandboxed AI tasks.
