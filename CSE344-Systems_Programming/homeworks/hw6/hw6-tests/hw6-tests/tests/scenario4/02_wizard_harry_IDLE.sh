#!/usr/bin/env bash
# Scenario 4 - Terminal 3: wizard "harry".
# Sends NOTHING after ENROLL -> idle 10s -> server TIMEOUT DISCONNECT.
# Client prints "RECEIVED TIMEOUT DISCONNECT" then "DISCONNECTED reason=timeout".
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 4 - WIZARD harry  (IDLE - expect TIMEOUT)"
feed_and_wait \
    | ./wizard 127.0.0.1 5000 harry
