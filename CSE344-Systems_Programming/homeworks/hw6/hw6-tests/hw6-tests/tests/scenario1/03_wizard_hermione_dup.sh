#!/usr/bin/env bash
# Scenario 1 - Terminal 4: Duplicate wizard "hermione" (same username already enrolled).
# Expected: ENROLL hermione -> server responds "ERR ENROLL name_taken".
# Client just sits; user can Ctrl+C to leave.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 1 - WIZARD hermione (DUPLICATE)"
feed_and_wait \
    | ./wizard 127.0.0.1 5000 hermione
