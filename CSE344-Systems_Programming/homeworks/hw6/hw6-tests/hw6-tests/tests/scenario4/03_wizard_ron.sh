#!/usr/bin/env bash
# Scenario 4 - Terminal 4: wizard "ron".
# Connects later (after hermione's session is already running).
# Sends BREW UNICORN_HAIR 5, then idle until server SIGINT.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 4 - WIZARD ron"
feed_and_wait \
    "BREW UNICORN_HAIR 5" \
    | ./wizard 127.0.0.1 5000 ron
