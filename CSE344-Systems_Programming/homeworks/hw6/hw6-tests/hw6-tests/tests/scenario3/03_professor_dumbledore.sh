#!/usr/bin/env bash
# Scenario 3 - Terminal 4: professor "dumbledore".
# PDF: From dumbledore: ROSTER  (after voldemort/snape are connected and after neville reconnect)
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 3 - PROFESSOR dumbledore"
# Wait a bit so other clients are up first, then ROSTER.
sleep 2
feed_and_wait \
    "ROSTER" \
    | ./professor 127.0.0.1 5000 dumbledore
