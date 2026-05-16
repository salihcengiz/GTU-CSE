#!/usr/bin/env bash
# Scenario 3 - Terminal 5: wizard "voldemort".
# Server already at 3/3 clients (harry, ron, dumbledore) -> expect ERR HOGWARTS_FULL.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 3 - WIZARD voldemort  (expect ERR HOGWARTS_FULL)"
feed_and_wait \
    | ./wizard 127.0.0.1 5000 voldemort
