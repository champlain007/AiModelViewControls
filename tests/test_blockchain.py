import requests
import time
import sys
import os

def test_blockchain():
    base_url = "http://127.0.0.1:8080/api"
    
    # 1. Trigger Token Issue (Should record to blockchain)
    print("Issuing token to trigger blockchain record...")
    requests.post(f"{base_url}/token/issue", json={"owner": "blockchain_test_user", "scope": 0})
    
    # 2. Trigger Vault Tokenization (Should record to blockchain)
    print("Issuing INTERNAL token for vault access...")
    r = requests.post(f"{base_url}/token/issue", json={"owner": "admin", "scope": 0})
    admin_token = r.json()['token']
    
    print("Tokenizing secret...")
    headers = {"Authorization": f"Bearer {admin_token}"}
    requests.post(f"{base_url}/token/vault", headers=headers, json={"secret": "BlockchainSecret456"})

    # 3. Retrieve and Validate Ledger
    print("\nRetrieving Private Blockchain Ledger...")
    r = requests.get(f"{base_url}/ledger")
    ledger = r.json()
    
    print(f"Ledger Size: {len(ledger)} blocks")
    
    for block in ledger:
        print(f"Block #{block['index']} | Hash: {block['hash'][:16]}... | Prev: {block['prev_hash'][:16]}...")
        print(f"  Data: {block['data']}")

    if len(ledger) >= 3: # Genesis + TokenIssue + Vault
        print("\nSUCCESS: Private blockchain recorded multiple system transactions.")
        # Basic integrity check
        valid = True
        for i in range(1, len(ledger)):
            if ledger[i]['prev_hash'] != ledger[i-1]['hash']:
                valid = False
                break
        if valid:
            print("SUCCESS: Cryptographic chain integrity verified.")
        else:
            print("FAILURE: Chain integrity broken!")
    else:
        print(f"FAILURE: Ledger too small. Expected at least 3 blocks, found {len(ledger)}.")

if __name__ == "__main__":
    try:
        test_blockchain()
    except Exception as e:
        print(f"Error: {e}")
