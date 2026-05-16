#!/usr/bin/env bash
# Edge: client closes the TCP connection abruptly (no APPARATE).
# Server should detect hangup and log CLIENT_DISCONNECTED reason=hangup;
# the freed slot must be reusable (no fd leak).
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "EDGE - HANGUP (no APPARATE) + slot recovery"

# 1. Open a nc client that ENROLLs then is killed.
echo "[*] Opening abrupt-close nc client..."
{
    sleep 0.3
    echo "ENROLL WIZARD hangboi"
    sleep 0.5
    # die without APPARATE
} | nc -q 0 127.0.0.1 5000 &
NC_PID=$!
sleep 1.5
kill -9 $NC_PID 2>/dev/null || true
wait $NC_PID 2>/dev/null || true
echo "[*] nc client killed."
sleep 0.5

# 2. Open a new client with the same username -> must succeed (slot freed).
echo "[*] Reconnecting with the same username 'hangboi' -> should succeed."
feed_and_quit \
    "BREW MOONSTONE 1" \
    | ./wizard 127.0.0.1 5000 hangboi
