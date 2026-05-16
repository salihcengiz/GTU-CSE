#!/usr/bin/env bash
# Scenario 1 - Terminal 3: Professor "dumbledore".
# PDF says: From dumbledore: BREW MOONSTONE 5
#           From dumbledore: CONSUME MOONSTONE 5
#           From dumbledore: Ctrl+C
# BREW/CONSUME from a professor -> expect ERR UNAUTHORIZED.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 1 - PROFESSOR dumbledore"
feed_and_wait \
    "BREW MOONSTONE 5" \
    "CONSUME MOONSTONE 5" \
    | ./professor 127.0.0.1 5000 dumbledore
