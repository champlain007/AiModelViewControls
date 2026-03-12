import requests
import time
import sys
import os

def test_mcp_and_killswitch():
    hub_url = "http://127.0.0.1:8080/api"
    mcp_url = "http://127.0.0.1:9100/mcp/inference"
    
    # 1. Test MCP Tokenization
    print("Testing MCP Tokenization (Missing Token)...")
    r = requests.post(mcp_url, json={"prompt": "check files"})
    if "UNAUTHORIZED" in r.text:
        print("SUCCESS: MCP blocked unauthorized call.")
    else:
        print(f"FAILURE: MCP allowed unauthorized call! ({r.text})")

    # 2. Test MCP Sandboxing
    print("\nTesting MCP Sandboxing (Malicious Path)...")
    # Issue a valid token first
    r_token = requests.post(f"{hub_url}/token/issue", json={"owner": "mcp_test", "scope": 0})
    token = r_token.json()['token']
    
    headers = {"Authorization": f"Bearer {token}"}
    r = requests.post(mcp_url, headers=headers, json={"prompt": "check files"})
    if "SANDBOX_VIOLATION" in r.text:
        print("SUCCESS: MCP blocked access to /etc/shadow.")
    else:
        print(f"FAILURE: MCP allowed sandbox violation! ({r.text})")

    # 3. Test Emergency Kill Switch
    print("\nTriggering EMERGENCY KILL SWITCH...")
    r = requests.post(f"{hub_url}/security/killswitch")
    if r.status_code == 200:
        print("SUCCESS: Kill switch signal received.")
    
    # Verify isolation via audit log
    r_audit = requests.get(f"{hub_url}/audit")
    logs = r_audit.json()
    if any(l['action'] == "KILL_SWITCH" for l in logs):
        print("SUCCESS: Kill Switch event recorded in Audit Trail.")
    else:
        print("FAILURE: Kill Switch not logged.")

    # Verify system is in LOCKDOWN
    r_state = requests.get(f"{hub_url}/state")
    if r_state.json()['state'] == "LOCKDOWN":
        print("SUCCESS: System automatically entered LOCKDOWN mode.")
    else:
        print("FAILURE: System failed to enter LOCKDOWN.")

if __name__ == "__main__":
    # Note: Assumes mcp_inference_server.py is running on 9100
    try:
        test_mcp_and_killswitch()
    except Exception as e:
        print(f"Error: {e}")
