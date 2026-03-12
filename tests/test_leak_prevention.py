import requests
import time
import sys
import subprocess
import os

def start_orchestrator():
    print("Starting orchestrator...")
    return subprocess.Popen([sys.executable, "orchestrator.py"], cwd="AgenticMVCpipe")

def check_leak(url, payload):
    try:
        r = requests.post(f"{url}/api/send", json={
            "sender": "test_agent",
            "topic": "test",
            "content": payload
        }, timeout=2)
        return r.status_code, r.text
    except Exception as e:
        return None, str(e)

def test_leak_prevention():
    # Use a log file for the orchestrator to check its status
    with open("orchestrator_test.log", "w") as log_file:
        orchestrator = subprocess.Popen([sys.executable, "orchestrator.py"], 
                                      cwd="AgenticMVCpipe", 
                                      stdout=log_file, 
                                      stderr=subprocess.STDOUT)
    
    print("Waiting for nodes to start and be secured by orchestrator...")
    
    secured = False
    for i in range(60):
        if os.path.exists("AgenticMVCpipe/orchestrator_test.log"):
            with open("AgenticMVCpipe/orchestrator_test.log", "r") as f:
                content = f.read()
                if "MainHub is now hardened against secret leaks" in content:
                    print(f"Confirmed: MainHub is secured (attempt {i})")
                    secured = True
                    break
        time.sleep(1)
    
    if not secured:
        print("Timed out waiting for orchestrator to secure nodes.")
        orchestrator.terminate()
        return

    node_8080 = "http://127.0.0.1:8080"
    
    # Test 1: OpenAI Key Leak (Ingress)
    print("Testing OpenAI Key Leak (Ingress)...")
    status, text = check_leak(node_8080, "My secret key is sk-1234567890abcdef1234567890abcdef")
    if status == 403:
        print("SUCCESS: Ingress leak blocked correctly.")
    else:
        print(f"FAILURE: Ingress leak NOT blocked (Status: {status}, Response: {text})")

    # Test 2: Private Key Leak
    print("Testing Private Key Leak...")
    status, text = check_leak(node_8080, "-----BEGIN RSA PRIVATE KEY-----\nMIIEowIBAAKCAQEA75...")
    if status == 403:
        print("SUCCESS: Private key leak blocked correctly.")
    else:
        print(f"FAILURE: Private key leak NOT blocked (Status: {status})")

    # Test 3: Generic Secret Leak
    print("Testing Generic Secret Leak...")
    status, text = check_leak(node_8080, "api_key = 'super-secret-123456789'")
    if status == 403:
        print("SUCCESS: Generic secret leak blocked correctly.")
    else:
        print(f"FAILURE: Generic secret leak NOT blocked (Status: {status})")

    # Test 4: Root Password Leak
    print("Testing Root Password Leak...")
    status, text = check_leak(node_8080, "root_password = 'My_Super_Secret_Root_123!'")
    if status == 403:
        print("SUCCESS: Root password leak blocked correctly.")
    else:
        print(f"FAILURE: Root password leak NOT blocked (Status: {status})")

    # Test 5: Encrypted Private Key Leak
    print("Testing Encrypted Private Key Leak...")
    status, text = check_leak(node_8080, "-----BEGIN ENCRYPTED PRIVATE KEY-----\nMIIFDjBABgkqhkiG9w0BBQ0wMzAbBgkqhkiG9w0BBQwwDgQI...")
    if status == 403:
        print("SUCCESS: Encrypted private key leak blocked correctly.")
    else:
        print(f"FAILURE: Encrypted private key leak NOT blocked (Status: {status})")

    # Check alerts
    print("Checking system alerts for leaks...")
    try:
        r = requests.get(f"{node_8080}/api/state")
        alerts = r.json().get('alerts', [])
        leak_alerts = [a for a in alerts if "BLOCK" in a['type'] or "LEAK" in a['type']]
        if leak_alerts:
            print(f"SUCCESS: Found {len(leak_alerts)} leak/block alerts in system state.")
            for a in leak_alerts:
                print(f"  - Alert: {a['details']}")
        else:
            print("FAILURE: No leak alerts found in system state.")
    except Exception as e:
        print(f"Error checking alerts: {e}")

    orchestrator.terminate()

if __name__ == "__main__":
    test_leak_prevention()
