#!/usr/bin/env bash
# Scenario 3 - Terminal 3: wizard "ron". User will Ctrl+C after dumbledore's ROSTER.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 3 - WIZARD ron"
feed_and_wait \
    | ./wizard 127.0.0.1 5000 ron
