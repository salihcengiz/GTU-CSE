#!/usr/bin/env bash
# Edge: normal APPARATE flow (client typed APPARATE itself, no signal).
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "EDGE - normal APPARATE exit"
feed_and_quit \
    "BREW PHOENIX_FEATHER 3" \
    "SPELLBOOK" \
    | ./wizard 127.0.0.1 5000 apparate_w
