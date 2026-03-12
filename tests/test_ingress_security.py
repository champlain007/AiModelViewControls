import requests
import time
import sys
import os

def test_ingress_security():
    base_url = "http://127.0.0.1:8080/api"
    
    # 1. Simulate Malware Ingress (EICAR) via /api/send
    print("Testing Ingress Malware (EICAR Signature) via /api/send...")
    eicar = "X5O!P%@AP[4\\PZX54(P^)7CC)7}$EICAR-STANDARD-ANTIVIRUS-TEST-FILE!$H+H*"
    
    payload = {
        "sender": "external_source",
        "topic": "ingress_test",
        "content": eicar
    }
    r = requests.post(f"{base_url}/send", json=payload)
    
    if r.status_code == 403 and "SECURITY_BLOCK" in r.text:
        print("SUCCESS: Ingress malware blocked correctly.")
    else:
        print(f"FAILURE: Ingress malware NOT blocked (Status: {r.status_code}, Response: {r.text})")

    # 2. Check Quarantine
    print("Checking Quarantine for ingress malware...")
    r = requests.get(f"{base_url}/quarantine")
    q_list = r.json()
    
    if any("EICAR" in q['reason'] and q['origin'] == "external_source" for q in q_list):
        print(f"SUCCESS: Ingress malware quarantined correctly.")
    else:
        print("FAILURE: Ingress malware NOT found in quarantine.")

    # 3. Simulate Malware via File Ingress (if possible)
    # We'd need to create a file and add a HubConnector for it.
    # For now, /api/send coverage is a good start for ingress.

if __name__ == "__main__":
    try:
        test_ingress_security()
    except Exception as e:
        print(f"Error: {e}")
