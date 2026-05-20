#!/usr/bin/env python3
"""Simple JSON field extractor for smoke test piping."""
import json, sys
d = json.load(sys.stdin)
keys = sys.argv[1:] if len(sys.argv) > 1 else list(d.keys())
parts = []
for k in keys:
    v = d.get(k, "N/A")
    parts.append(f"{k}={v}")
print(", ".join(parts))
