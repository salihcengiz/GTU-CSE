#!/usr/bin/env bash
# Scenario 3 - Terminal 6: professor "snape".
# Capacity still full -> expect ERR HOGWARTS_FULL.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 3 - PROFESSOR snape  (expect ERR HOGWARTS_FULL)"
feed_and_wait \
    | ./professor 127.0.0.1 5000 snape
