#!/usr/bin/env bash
# Scenario 1 - Terminal 2: First wizard "hermione".
# PDF says: From hermione: FLY BROOMSTICK
#           From hermione: INSPECT MOONSTONE
#           From hermione: SCROLL
#           From hermione: Ctrl+C
# Both INSPECT and SCROLL are professor-only -> expect ERR UNAUTHORIZED.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 1 - WIZARD hermione"
feed_and_wait \
    "FLY BROOMSTICK" \
    "INSPECT MOONSTONE" \
    "SCROLL" \
    | ./wizard 127.0.0.1 5000 hermione
