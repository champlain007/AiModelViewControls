import requests
import time
import sys
import os

def test_security_suite():
    base_url = "http://127.0.0.1:8080/api"
    
    # 1. Simulate Malware Egress (EICAR)
    print("Testing Malware Egress (EICAR Signature)...")
    eicar = "X5O!P%@AP[4\\PZX54(P^)7CC)7}$EICAR-STANDARD-ANTIVIRUS-TEST-FILE!$H+H*"
    
    # We need a connector to send through. 
    # Create a webhook connector if not exists (or rely on existing)
    # Let's create a specific one for testing
    setup = {
        "id": "SecurityTestNet",
        "type": 2, # URL_STREAM
        "target": "http://example.com/upload",
        "sandbox": { "level": 1, "allowed_domains": ["example.com"], "allow_network": True }
    }
    requests.post(f"{base_url}/hub", json=setup)
    
    # Send Malware
    payload = {
        "sender": "malicious_actor",
        "topic": "upload",
        "content": eicar
    }
    requests.post(f"{base_url}/send", json=payload)
    
    time.sleep(2) # Allow async processing

    # 2. Check Quarantine
    print("Checking Quarantine...")
    r = requests.get(f"{base_url}/quarantine")
    q_list = r.json()
    
    quarantine_id = ""
    if any("EICAR" in q['reason'] for q in q_list):
        print(f"SUCCESS: Malware quarantined successfully. Found {len(q_list)} items.")
        quarantine_id = q_list[0]['id']
    else:
        print("FAILURE: Malware NOT found in quarantine.")
        return

    # 3. Check Audit Log
    print("Checking Audit Log...")
    r = requests.get(f"{base_url}/audit")
    logs = r.json()
    
    if any(l['action'] == "QUARANTINE" for l in logs):
        print("SUCCESS: Quarantine event logged in Audit Trail.")
    else:
        print("FAILURE: No audit log found for quarantine.")

    # 4. Manage Quarantine (Release/Delete)
    if quarantine_id:
        print(f"Releasing Quarantine Item {quarantine_id}...")
        r = requests.post(f"{base_url}/quarantine/manage", json={"id": quarantine_id, "action": "release"})
        if r.status_code == 200:
            print("SUCCESS: Item released.")
        else:
            print(f"FAILURE: Release failed ({r.status_code})")

        print(f"Deleting Quarantine Item {quarantine_id}...")
        r = requests.post(f"{base_url}/quarantine/manage", json={"id": quarantine_id, "action": "delete"})
        if r.status_code == 200:
            print("SUCCESS: Item deleted.")
        else:
            print(f"FAILURE: Delete failed ({r.status_code})")

    # 5. Verify Deletion
    r = requests.get(f"{base_url}/quarantine")
    if len(r.json()) == 0:
        print("SUCCESS: Quarantine is empty.")
    else:
        print("FAILURE: Quarantine not empty after delete.")

if __name__ == "__main__":
    try:
        test_security_suite()
    except Exception as e:
        print(f"Error: {e}")
