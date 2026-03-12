#!/bin/bash
# ==============================================================================
# AgenticMVCpipe - Trade Secret Configuration and Boot Script
# ==============================================================================
# This script is the ONLY configuration and initiating (booting) file for
# handshaking, transfers, and connections between AgenticMVCpipe instances.
# ==============================================================================
export TRADESECRET_HANDSHAKE_KEY="APPROVED_BY_TRADESECRET"
export ORCH_PORT=${ORCH_PORT:-9000}
export MCP_PORT=${MCP_PORT:-9100}
export WS_PORT=${WS_PORT:-9200}
export PORT=${PORT:-8080}
BINARY="./build/AgenticPipeline"
if [ ! -f "$BINARY" ]; then mkdir -p build && cd build && cmake .. && make && cd ..; fi
if [ "$1" == "--interactive" ] || [ "$1" == "-i" ]; then
    read -p "Enter Orchestrator Port [$ORCH_PORT]: " user_orch
    export ORCH_PORT=${user_orch:-$ORCH_PORT}
    read -p "Enter MCP Gateway Port [$MCP_PORT]: " user_mcp
    export MCP_PORT=${user_mcp:-$MCP_PORT}
    read -p "Enter WebSocket Sandbox Port [$WS_PORT]: " user_ws
    export WS_PORT=${user_ws:-$WS_PORT}
    read -p "Enter Dashboard HTTP Port [$PORT]: " user_port
    export PORT=${user_port:-$PORT}
    read -p "Enable Headless mode? (y/N): " user_headless
    if [[ "$user_headless" =~ ^[Yy]$ ]]; then HEADLESS_FLAG="--headless"; fi
fi
$BINARY --orchestrator &
ORCH_PID=$!
$BINARY --mcp &
MCP_PID=$!
$BINARY --ws-sandbox &
WS_PID=$!
trap 'kill $ORCH_PID $MCP_PID $WS_PID 2>/dev/null; exit 0' SIGINT SIGTERM EXIT
$BINARY $HEADLESS_FLAG
