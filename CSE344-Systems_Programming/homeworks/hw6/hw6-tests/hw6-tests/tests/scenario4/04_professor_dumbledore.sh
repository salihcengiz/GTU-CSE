#!/usr/bin/env bash
# Scenario 4 - Terminal 5: professor "dumbledore".
# SCROLL after ron's BREW so the global table reflects updates.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 4 - PROFESSOR dumbledore"
feed_and_wait \
    "SCROLL" \
    | ./professor 127.0.0.1 5000 dumbledore
