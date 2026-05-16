#!/usr/bin/env bash
# Scenario 3 - Terminal 2: wizard "harry". Will be Ctrl+C'd to free a slot.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 3 - WIZARD harry  (Ctrl+C BEFORE neville connects)"
feed_and_wait \
    | ./wizard 127.0.0.1 5000 harry
