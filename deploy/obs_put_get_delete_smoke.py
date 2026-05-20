#!/usr/bin/env python3
"""OBS PUT/GET/DELETE smoke test for Round 22.
Uses environment variables OBJECT_STORAGE_ACCESS_KEY_ID and OBJECT_STORAGE_SECRET_ACCESS_KEY.
Does NOT print credentials.
"""
import hashlib, os, sys, time
from datetime import datetime, timezone
import urllib.request, urllib.error, urllib.parse
import hmac, base64, xml.etree.ElementTree as ET

ENDPOINT = "obs.cn-north-4.myhuaweicloud.com"
REGION = "cn-north-4"
BUCKET = "20260518-bighomework"
TEST_PREFIX = "technical-spike/round22"
TEST_CONTENT = f"Round22 OBS smoke test - {datetime.now(timezone.utc).isoformat()}"
TEST_KEY = f"{TEST_PREFIX}/{hashlib.sha256(TEST_CONTENT.encode()).hexdigest()[:16]}.txt"

AK = os.environ.get("OBJECT_STORAGE_ACCESS_KEY_ID", "")
SK = os.environ.get("OBJECT_STORAGE_SECRET_ACCESS_KEY", "")
if not AK or not SK:
    print("ERROR: OBJECT_STORAGE_ACCESS_KEY_ID or OBJECT_STORAGE_SECRET_ACCESS_KEY not set")
    sys.exit(1)

def sign(method, key, content_type="", content=b"", headers_extra=None):
    date = datetime.now(timezone.utc).strftime("%a, %d %b %Y %H:%M:%S GMT")
    resource = f"/{BUCKET}/{key}"
    string_to_sign = f"{method}\n\n{content_type}\n{date}\n{resource}"
    signature = base64.b64encode(
        hmac.new(SK.encode(), string_to_sign.encode(), hashlib.sha1).digest()
    ).decode()
    url = f"https://{BUCKET}.{ENDPOINT}/{key}"
    req = urllib.request.Request(url, data=content if content else None, method=method)
    req.add_header("Date", date)
    req.add_header("Authorization", f"AWS {AK}:{signature}")
    if content_type:
        req.add_header("Content-Type", content_type)
    if headers_extra:
        for k, v in headers_extra.items():
            req.add_header(k, v)
    return req

# PUT
print(f"=== PUT {TEST_KEY} ===")
req = sign("PUT", TEST_KEY, "text/plain", TEST_CONTENT.encode())
try:
    resp = urllib.request.urlopen(req)
    print(f"PUT status: {resp.status}")
except urllib.error.HTTPError as e:
    print(f"PUT failed: {e.code} {e.reason}")
    sys.exit(1)

# GET
print(f"=== GET {TEST_KEY} ===")
req = sign("GET", TEST_KEY)
try:
    resp = urllib.request.urlopen(req)
    body = resp.read()
    got_hash = hashlib.sha256(body).hexdigest()
    expected_hash = hashlib.sha256(TEST_CONTENT.encode()).hexdigest()
    print(f"GET status: {resp.status}")
    print(f"SHA-256 match: {got_hash == expected_hash}")
    if got_hash != expected_hash:
        print(f"Expected: {expected_hash}")
        print(f"Got:      {got_hash}")
        sys.exit(1)
except urllib.error.HTTPError as e:
    print(f"GET failed: {e.code} {e.reason}")
    sys.exit(1)

# DELETE
print(f"=== DELETE {TEST_KEY} ===")
req = sign("DELETE", TEST_KEY)
try:
    resp = urllib.request.urlopen(req)
    print(f"DELETE status: {resp.status}")
except urllib.error.HTTPError as e:
    print(f"DELETE status: {e.code} (expected for DELETE)")

# Verify deleted
print(f"=== Verify deleted: GET {TEST_KEY} (expect 404) ===")
req = sign("GET", TEST_KEY)
try:
    resp = urllib.request.urlopen(req)
    print(f"ERROR: Object still exists after DELETE, status: {resp.status}")
    sys.exit(1)
except urllib.error.HTTPError as e:
    if e.code == 404:
        print(f"Confirmed deleted: {e.code}")
    else:
        print(f"Unexpected: {e.code} {e.reason}")
        sys.exit(1)

print("\n=== OBS PUT/GET/DELETE smoke test PASSED ===")
print(f"Test key prefix: {TEST_PREFIX}/")
print(f"SHA-256: {expected_hash}")
