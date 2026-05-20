#!/bin/bash
set -e
SERVER=http://127.0.0.1:8080

LOGIN_JSON=$(curl -s -X POST $SERVER/api/auth/login -H 'Content-Type: application/json' -d '{"campusEmail":"smoketest@campus.edu.cn","password":"SmokeTest123!"}')
TOKEN=$(echo "$LOGIN_JSON" | python3 -c 'import sys,json; print(json.load(sys.stdin)["accessToken"])')
USER_ID=a0eebc99-9c0b-4ef8-bb6d-6bb9bd380a11

echo "=== 2. GET /api/me/credit-summary ==="
curl -s $SERVER/api/me/credit-summary -H "Authorization: Bearer $TOKEN"
echo ""

echo "=== 3. GET /api/users/$USER_ID/credit-summary (public) ==="
curl -s $SERVER/api/users/$USER_ID/credit-summary -H "Authorization: Bearer $TOKEN"
echo ""

echo "=== 4. GET /api/me/reviews/given ==="
curl -s "$SERVER/api/me/reviews/given" -H "Authorization: Bearer $TOKEN"
echo ""

echo "=== 5. GET /api/me/reviews/received ==="
curl -s "$SERVER/api/me/reviews/received" -H "Authorization: Bearer $TOKEN"
echo ""

echo "=== 6. No auth (expect 401) ==="
curl -s -o /dev/null -w "HTTP %{http_code}" $SERVER/api/me/credit-summary
echo ""
