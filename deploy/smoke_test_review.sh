#!/bin/bash
set -e
SERVER=http://127.0.0.1:8080
USER1_ID=a0eebc99-9c0b-4ef8-bb6d-6bb9bd380a11
USER2_ID=b0eebc99-9c0b-4ef8-bb6d-6bb9bd380a22

# Login as user1
LOGIN_JSON=$(curl -s -X POST $SERVER/api/auth/login -H 'Content-Type: application/json' -d '{"campusEmail":"smoketest@campus.edu.cn","password":"SmokeTest123!"}')
TOKEN=$(echo "$LOGIN_JSON" | python3 -c 'import sys,json; print(json.load(sys.stdin)["accessToken"])')

echo "=== 7. POST /api/me/reviews (submit review) ==="
REVIEW_RESP=$(curl -s -X POST $SERVER/api/me/reviews -H "Authorization: Bearer $TOKEN" -H 'Content-Type: application/json' -d "{\"conversationId\":1,\"revieweeId\":\"$USER2_ID\",\"rating\":5,\"comment\":\"Nice person!\"}")
echo "$REVIEW_RESP"
echo ""

echo "=== 8. GET /api/me/credit-summary (after review) ==="
curl -s $SERVER/api/me/credit-summary -H "Authorization: Bearer $TOKEN"
echo ""

echo "=== 9. GET /api/me/reviews/given (after review) ==="
curl -s "$SERVER/api/me/reviews/given" -H "Authorization: Bearer $TOKEN"
echo ""

echo "=== 10. GET /api/users/$USER2_ID/credit-summary (reviewee public summary) ==="
curl -s $SERVER/api/users/$USER2_ID/credit-summary -H "Authorization: Bearer $TOKEN"
echo ""
