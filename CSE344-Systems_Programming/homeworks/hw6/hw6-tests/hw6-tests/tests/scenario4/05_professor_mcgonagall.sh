#!/usr/bin/env bash
# Scenario 4 - Terminal 6: professor "mcgonagall".
# Sends ROSTER; then idle until server SIGINT triggers SERVER SHUTDOWN.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 4 - PROFESSOR mcgonagall"
feed_and_wait \
    "ROSTER" \
    | ./professor 127.0.0.1 5000 mcgonagall
