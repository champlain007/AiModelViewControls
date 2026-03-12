import requests
import time
import sys
import os

def test_tokenization():
    base_url = "http://127.0.0.1:8080/api"
    
    # 1. Issue an INTERNAL token
    print("Issuing INTERNAL token...")
    r = requests.post(f"{base_url}/token/issue", json={"owner": "test_admin", "scope": 0})
    if r.status_code != 200:
        print(f"FAILED to issue token: {r.text}")
        return
    
    admin_token = r.json()['token']
    print(f"SUCCESS: Issued token {admin_token[:8]}...")

    # 2. Issue an EXTERNAL token
    print("Issuing EXTERNAL token...")
    r = requests.post(f"{base_url}/token/issue", json={"owner": "external_service", "scope": 1})
    ext_token = r.json()['token']
    print(f"SUCCESS: Issued token {ext_token[:8]}...")

    # 3. Test Vault Access with Admin Token (Tokenize)
    print("Testing Vault (Tokenize) with Admin Token...")
    headers = {"Authorization": f"Bearer {admin_token}"}
    r = requests.post(f"{base_url}/token/vault", headers=headers, json={"secret": "SuperSecretPassword123"})
    if r.status_code == 200:
        vault_id = r.json()['token_id']
        print(f"SUCCESS: Secret tokenized to {vault_id}")
    else:
        print(f"FAILED vault access: {r.status_code} {r.text}")
        return

    # 4. Test Vault Access with External Token (Should FAIL detokenize)
    print("Testing Vault (Detokenize) with External Token (Should Fail)...")
    headers_ext = {"Authorization": f"Bearer {ext_token}"}
    r = requests.post(f"{base_url}/token/vault", headers=headers_ext, json={"token_id": vault_id})
    if r.status_code == 401:
        print("SUCCESS: Unauthorized access blocked correctly.")
    else:
        print(f"FAILURE: Unauthorized access NOT blocked (Status: {r.status_code})")

    # 5. Test Vault Access with Admin Token (Detokenize)
    print("Testing Vault (Detokenize) with Admin Token...")
    r = requests.post(f"{base_url}/token/vault", headers=headers, json={"token_id": vault_id})
    if r.status_code == 200:
        secret = r.json()['secret']
        if secret == "SuperSecretPassword123":
            print(f"SUCCESS: Detokenized secret matches original.")
        else:
            print(f"FAILURE: Detokenized secret mismatch: {secret}")
    else:
        print(f"FAILED detokenize: {r.status_code} {r.text}")

if __name__ == "__main__":
    # Ensure orchestrator is running (assuming it's already started from previous turns or manually)
    # For a clean test, we could start it here, but let's assume environment is ready.
    try:
        test_tokenization()
    except Exception as e:
        print(f"Error running test: {e}")
