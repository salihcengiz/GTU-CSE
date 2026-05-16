#!/usr/bin/env bash
# Scenario 1 - Terminal 5: Third client, send a command BEFORE ENROLL.
# We use `nc` to bypass the wizard binary's automatic ENROLL.
# Expected: server replies "ERR NOT_ENROLLED" and we still stay connected.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 1 - RAW CLIENT (command before ENROLL)"

{
    sleep 0.5
    echo "BREW MOONSTONE 1"     # should get ERR NOT_ENROLLED
    sleep "$SLEEP"
    echo "INSPECT MOONSTONE"    # also NOT_ENROLLED
    sleep "$SLEEP"
    # Then leave the connection open for manual Ctrl+C.
    cat
} | nc 127.0.0.1 5000
