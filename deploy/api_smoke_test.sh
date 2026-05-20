#!/bin/bash
set -euo pipefail
BASE="http://localhost:8080"
JE="python3 /srv/campus-buddy/json_extract.py"

echo "=== 1. Health ==="
curl -sf "$BASE/api/health" | $JE status || { echo "FAIL: health"; exit 1; }

echo "=== 2. System Info ==="
curl -sf "$BASE/api/system/info" | $JE serviceName version || { echo "FAIL: system-info"; exit 1; }

echo "=== 3. Login ==="
LOGIN_RESP=$(curl -sf -X POST "$BASE/api/auth/login" \
  -H "Content-Type: application/json" \
  -d '{"campusEmail":"smoketest@campus.edu.cn","password":"SmokeTest123!"}')
TOKEN=$(echo "$LOGIN_RESP" | $JE accessToken | cut -d= -f2)
echo "Token acquired: ${#TOKEN} chars"

echo "=== 4. My Credit Summary ==="
curl -sf -H "Authorization: Bearer $TOKEN" "$BASE/api/me/credit-summary" | $JE creditScore reviewCount || { echo "FAIL: credit-summary"; exit 1; }

echo "=== 5. Reviews Given ==="
curl -sf -H "Authorization: Bearer $TOKEN" "$BASE/api/me/reviews/given?page=0&size=5" | $JE totalElements || { echo "FAIL: reviews given"; exit 1; }

echo "=== 6. Secure Probe (auth) ==="
curl -sf -H "Authorization: Bearer $TOKEN" "$BASE/api/probe/secure" | $JE authenticated || { echo "FAIL: secure probe"; exit 1; }

echo "=== 7. Secure Probe (no auth, expect 401) ==="
CODE=$(curl -s -o /dev/null -w '%{http_code}' "$BASE/api/probe/secure")
echo "Status: $CODE"
[ "$CODE" = "401" ] || { echo "FAIL: expected 401, got $CODE"; exit 1; }

echo "=== 8. Public user credit summary (auth required) ==="
USER_ID="a0eebc99-9c0b-4ef8-bb6d-6bb9bd380a11"
curl -sf -H "Authorization: Bearer $TOKEN" "$BASE/api/users/$USER_ID/credit-summary" | $JE creditScore || { echo "FAIL: public summary"; exit 1; }

echo ""
echo "=== ALL API SMOKE TESTS PASSED ==="
