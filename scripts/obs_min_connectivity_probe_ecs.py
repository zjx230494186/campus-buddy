#!/usr/bin/env python3
import datetime as dt
import hashlib
import hmac
import os
import socket
import ssl
import sys
import urllib.parse
import urllib.request
import uuid


ENDPOINT = os.getenv("OBJECT_STORAGE_ENDPOINT", "obs.cn-north-4.myhuaweicloud.com")
REGION = os.getenv("OBJECT_STORAGE_REGION", "cn-north-4")
BUCKET = os.getenv("OBJECT_STORAGE_BUCKET", "20260518-bighomework")
KEY_PREFIX = os.getenv("OBJECT_STORAGE_PROBE_PREFIX", "technical-spike/min-connectivity-ecs")


def first_env(names):
    for name in names:
        value = os.getenv(name)
        if value and value.strip():
            return name, value
    return None, None


def sha256_hex(data):
    return hashlib.sha256(data).hexdigest()


def hmac_sha256(key, message):
    return hmac.new(key, message.encode("utf-8"), hashlib.sha256).digest()


def signing_key(secret_key, date_stamp, region, service):
    key = ("AWS4" + secret_key).encode("utf-8")
    key = hmac_sha256(key, date_stamp)
    key = hmac_sha256(key, region)
    key = hmac_sha256(key, service)
    return hmac_sha256(key, "aws4_request")


def check_tcp_tls(host):
    print(f"Network check host: {host}:443")
    with socket.create_connection((host, 443), timeout=10) as sock:
        with ssl.create_default_context().wrap_socket(sock, server_hostname=host) as tls_sock:
            print(f"TLS version: {tls_sock.version()}")


def signed_request(method, uri, body, access_key, secret_key, token):
    service = "s3"
    host = f"{BUCKET}.{ENDPOINT}"
    now = dt.datetime.now(dt.timezone.utc)
    amz_date = now.strftime("%Y%m%dT%H%M%SZ")
    date_stamp = now.strftime("%Y%m%d")
    payload_hash = sha256_hex(body)

    canonical_headers = (
        f"host:{host}\n"
        f"x-amz-content-sha256:{payload_hash}\n"
        f"x-amz-date:{amz_date}\n"
    )
    signed_headers = "host;x-amz-content-sha256;x-amz-date"
    headers = {
        "x-amz-content-sha256": payload_hash,
        "x-amz-date": amz_date,
    }

    if token:
        canonical_headers += f"x-amz-security-token:{token}\n"
        signed_headers += ";x-amz-security-token"
        headers["x-amz-security-token"] = token

    canonical_request = "\n".join([
        method,
        uri,
        "",
        canonical_headers,
        signed_headers,
        payload_hash,
    ])
    credential_scope = f"{date_stamp}/{REGION}/{service}/aws4_request"
    string_to_sign = "\n".join([
        "AWS4-HMAC-SHA256",
        amz_date,
        credential_scope,
        sha256_hex(canonical_request.encode("utf-8")),
    ])
    signature = hmac.new(
        signing_key(secret_key, date_stamp, REGION, service),
        string_to_sign.encode("utf-8"),
        hashlib.sha256,
    ).hexdigest()

    headers["Authorization"] = (
        "AWS4-HMAC-SHA256 "
        f"Credential={access_key}/{credential_scope}, "
        f"SignedHeaders={signed_headers}, Signature={signature}"
    )
    if body:
        headers["Content-Type"] = "text/plain; charset=utf-8"

    req = urllib.request.Request(f"https://{host}{uri}", data=body if body else None, headers=headers, method=method)
    with urllib.request.urlopen(req, timeout=20) as response:
        return response.status, response.read()


def main():
    access_name, access_key = first_env([
        "OBJECT_STORAGE_ACCESS_KEY_ID",
        "OBS_ACCESS_KEY_ID",
        "HUAWEICLOUD_OBS_ACCESS_KEY_ID",
        "AWS_ACCESS_KEY_ID",
    ])
    secret_name, secret_key = first_env([
        "OBJECT_STORAGE_SECRET_ACCESS_KEY",
        "OBS_SECRET_ACCESS_KEY",
        "HUAWEICLOUD_OBS_SECRET_ACCESS_KEY",
        "AWS_SECRET_ACCESS_KEY",
    ])
    token_name, token = first_env([
        "OBJECT_STORAGE_SESSION_TOKEN",
        "OBS_SESSION_TOKEN",
        "HUAWEICLOUD_OBS_SESSION_TOKEN",
        "AWS_SESSION_TOKEN",
    ])

    print("ECS OBS minimal connectivity probe started.")
    print(f"Endpoint: {ENDPOINT}")
    print(f"Region: {REGION}")
    print(f"Bucket: {BUCKET}")
    check_tcp_tls(ENDPOINT)

    if not access_key or not secret_key:
        print("Probe skipped: missing server-side credentials in environment.")
        print("Accepted access key env names: OBJECT_STORAGE_ACCESS_KEY_ID, OBS_ACCESS_KEY_ID, HUAWEICLOUD_OBS_ACCESS_KEY_ID, AWS_ACCESS_KEY_ID")
        print("Accepted secret key env names: OBJECT_STORAGE_SECRET_ACCESS_KEY, OBS_SECRET_ACCESS_KEY, HUAWEICLOUD_OBS_SECRET_ACCESS_KEY, AWS_SECRET_ACCESS_KEY")
        print("Optional session token env names: OBJECT_STORAGE_SESSION_TOKEN, OBS_SESSION_TOKEN, HUAWEICLOUD_OBS_SESSION_TOKEN, AWS_SESSION_TOKEN")
        return 2

    object_key = f"{KEY_PREFIX}/{uuid.uuid4().hex}.txt"
    object_uri = "/" + urllib.parse.quote(object_key, safe="/")
    payload = (
        "campus-buddy ECS OBS minimal connectivity probe\n"
        f"createdUtc={dt.datetime.now(dt.timezone.utc).isoformat()}\n"
    ).encode("utf-8")
    expected_hash = sha256_hex(payload)

    credential_names = f"{access_name}, {secret_name}"
    if token_name:
        credential_names += f", {token_name}"
    print(f"Credential env names: {credential_names}")
    print(f"Object key: {object_key}")

    put_status, _ = signed_request("PUT", object_uri, payload, access_key, secret_key, token)
    print(f"PUT status: {put_status}")

    get_status, downloaded = signed_request("GET", object_uri, b"", access_key, secret_key, token)
    print(f"GET status: {get_status}")

    actual_hash = sha256_hex(downloaded)
    if actual_hash != expected_hash:
        raise RuntimeError(f"Downloaded object hash mismatch. Expected {expected_hash}, got {actual_hash}.")
    print(f"SHA-256 verified: {actual_hash}")

    delete_status, _ = signed_request("DELETE", object_uri, b"", access_key, secret_key, token)
    print(f"DELETE status: {delete_status}")
    print("ECS OBS minimal connectivity probe passed.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print("ECS OBS minimal connectivity probe failed.")
        print(str(exc))
        raise SystemExit(1)
