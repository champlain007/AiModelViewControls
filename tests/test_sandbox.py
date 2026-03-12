import requests
import time
import sys
import os

def test_sandbox():
    base_url = "http://127.0.0.1:8080/api"
    
    # 1. Add a Connector with PARANOID sandbox (No access allowed)
    print("Adding PARANOID connector (Restricted to /tmp/safe)...")
    payload = {
        "id": "SandboxedReader",
        "type": 1, # LOCAL_FILE
        "target": "/etc/passwd", # Attempt to read sensitive file
        "sandbox": {
            "level": 3, # PARANOID
            "allowed_paths": ["/tmp/safe"],
            "allow_network": False,
            "allow_disk_write": False
        }
    }
    r = requests.post(f"{base_url}/hub", json=payload)
    print(f"Response: {r.text}")

    # 2. Check alerts for sandbox violation
    print("Waiting for RouterEngine to attempt read...")
    time.sleep(5)
    
    r = requests.get(f"{base_url}/state")
    alerts = r.json().get('alerts', [])
    sandbox_alerts = [a for a in alerts if "SANDBOX_VIOLATION" in a['type']]
    
    if sandbox_alerts:
        print(f"SUCCESS: Found {len(sandbox_alerts)} sandbox violations.")
        for a in sandbox_alerts:
            print(f"  - Alert: {a['details']}")
    else:
        print("FAILURE: No sandbox violation alerts found (Did it read /etc/passwd? Check orchestrator logs).")

    # 3. Test Network Sandboxing
    print("\nAdding Sandboxed Webhook (Restricted to safe.com)...")
    payload_net = {
        "id": "SandboxedNet",
        "type": 2, # URL_STREAM
        "target": "http://malicious-site.com/leak", 
        "sandbox": {
            "level": 2, # RESTRICTED
            "allowed_domains": ["safe.com"],
            "allow_network": True
        }
    }
    requests.post(f"{base_url}/hub", json=payload_net)
    
    time.sleep(5)
    r = requests.get(f"{base_url}/state")
    alerts = r.json().get('alerts', [])
    net_alerts = [a for a in alerts if "malicious-site.com" in a['details']]
    
    if net_alerts:
        print(f"SUCCESS: Network sandbox blocked malicious domain.")
    else:
        print("FAILURE: Network sandbox did NOT block malicious domain.")

if __name__ == "__main__":
    test_sandbox()
