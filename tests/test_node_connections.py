import requests
import time
import sys

def check_node(url):
    try:
        r = requests.get(f"{url}/api/state", timeout=2)
        return r.status_code == 200
    except:
        return False

def get_messages(url):
    try:
        r = requests.get(f"{url}/api/state", timeout=2)
        if r.status_code == 200:
            return r.json().get('messages', [])
    except:
        pass
    return []

def send_message(url, sender, topic, content):
    payload = {
        "sender": sender,
        "topic": topic,
        "content": content
    }
    try:
        r = requests.post(f"{url}/api/send", json=payload, timeout=2)
        return r.status_code == 200
    except Exception as e:
        print(f"Error sending to {url}: {e}")
        return False

def test_connections():
    node_8081 = "http://127.0.0.1:8081"
    node_8083 = "http://127.0.0.1:8083"

    print(f"Checking if nodes are online...")
    for _ in range(30):
        if check_node(node_8081) and check_node(node_8083):
            print("Both nodes 8081 and 8083 are online.")
            break
        print("Waiting for nodes...")
        time.sleep(2)
    else:
        print("Nodes failed to come online.")
        sys.exit(1)

    # Test 8081 -> 8083
    test_content = f"Test message from 8081 at {time.time()}"
    print(f"Sending message to 8081: {test_content}")
    if send_message(node_8081, "test_8081", "test_topic", test_content):
        print("Message sent to 8081. Waiting for propagation to 8083...")
        for _ in range(10):
            messages = get_messages(node_8083)
            if any(m['content'] == test_content for m in messages):
                print("SUCCESS: Message propagated from 8081 to 8083.")
                break
            time.sleep(1)
        else:
            print("FAILURE: Message did not propagate from 8081 to 8083.")
            # sys.exit(1)
    else:
        print("FAILURE: Could not send message to 8081.")

    # Test 8083 -> 8081
    test_content_reverse = f"Test message from 8083 at {time.time()}"
    print(f"Sending message to 8083: {test_content_reverse}")
    if send_message(node_8083, "test_8083", "test_topic", test_content_reverse):
        print("Message sent to 8083. Waiting for propagation to 8081...")
        for _ in range(10):
            messages = get_messages(node_8081)
            if any(m['content'] == test_content_reverse for m in messages):
                print("SUCCESS: Message propagated from 8083 to 8081.")
                break
            time.sleep(1)
        else:
            print("FAILURE: Message did not propagate from 8083 to 8081.")
            # sys.exit(1)
    else:
        print("FAILURE: Could not send message to 8083.")

if __name__ == "__main__":
    test_connections()
