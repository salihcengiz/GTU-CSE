#!/usr/bin/env bash
# Scenario 3 - Terminal 7: wizard "neville".
# RUN THIS *AFTER* you have Ctrl+C'd harry in terminal 2 -> a slot opens up,
# so neville must connect successfully and BREW.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 3 - WIZARD neville  (only after harry Ctrl+C'd)"
feed_and_wait \
    "BREW MOONSTONE 5" \
    | ./wizard 127.0.0.1 5000 neville
