import requests
import time
import sys
import os

def test_orchestrator_control():
    orch_url = "http://127.0.0.1:9000/api"
    hub_url = "http://127.0.0.1:8080/api"
    
    # 1. Check Initial Status via Orchestrator
    print("Checking initial node status...")
    r = requests.get(f"{orch_url}/nodes/status")
    nodes = r.json()
    for n in nodes:
        print(f"Node {n['id']} | Online: {n['online']} | Isolated: {n['isolated']} | State: {n['state']}")

    # 2. Trigger Global Killswitch (Isolate ALL)
    print("\nTriggering GLOBAL KILLSWITCH (Isolate ALL)...")
    r = requests.post(f"{orch_url}/control/killswitch", json={"target_id": "ALL", "action": "ON"})
    print(f"Orchestrator Response: {r.json()}")

    time.sleep(2)

    # 3. Verify Isolation on MainHub
    print("\nVerifying Isolation on MainHub...")
    r = requests.get(f"{hub_url}/state")
    data = r.json()
    if data['isolated'] == True and data['state'] == "LOCKDOWN":
        print("SUCCESS: MainHub is isolated and in Lockdown.")
    else:
        print(f"FAILURE: MainHub status mismatch! {data}")

    # 4. Trigger Individual Reconnection via Orchestrator
    print("\nReconnecting MainHub individually...")
    r = requests.post(f"{orch_url}/control/killswitch", json={"target_id": "MainHub", "action": "OFF"})
    print(f"Orchestrator Response: {r.json()}")

    time.sleep(2)

    # 5. Final Verify
    r = requests.get(f"{hub_url}/state")
    data = r.json()
    if data['isolated'] == False and data['state'] == "NORMAL":
        print("SUCCESS: MainHub reconnected and state normalized.")
    else:
        print(f"FAILURE: Final status mismatch! {data}")

if __name__ == "__main__":
    try:
        test_orchestrator_control()
    except Exception as e:
        print(f"Error: {e}")
