import requests
import time
import random
import sys

API_URL = "http://127.0.0.1:8080/api"

agents = [
    {"id": "sensor_A", "type": "sensor"},
    {"id": "processor_X", "type": "cpu"},
    {"id": "logger_db", "type": "sink"}
]

topics = ["temperature", "cpu_load", "errors", "system"]

def register():
    for a in agents:
        try:
            requests.post(f"{API_URL}/register", json=a)
            print(f"Registered {a['id']}")
        except:
            print("Server not ready?")

def loop():
    while True:
        agent = random.choice(agents)
        topic = random.choice(topics)
        content = f"Value: {random.randint(0, 100)}"
        
        payload = {
            "sender": agent["id"],
            "topic": topic,
            "content": content
        }
        
        try:
            requests.post(f"{API_URL}/send", json=payload)
            print(f"Sent: {payload}")
        except Exception as e:
            print(f"Error sending: {e}")
            
        time.sleep(1)

if __name__ == "__main__":
    print("Starting Test Agent...")
    time.sleep(2) # Give server time to start
    register()
    loop()
