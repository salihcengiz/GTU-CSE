#!/usr/bin/env bash
# Scenario 4 - Terminal 2: wizard "hermione".
# Active wizard: sends BREW MOONSTONE 5, waits 3s, SPELLBOOK, waits 3s,
# BREW DRAGON_SCALE 2, waits 10s, SPELLBOOK.
# The 10s wait approaches the timeout boundary but each prior command resets it.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 4 - WIZARD hermione  (active)"
{
    sleep 0.5
    echo "BREW MOONSTONE 5";    sleep 3
    echo "SPELLBOOK";           sleep 3
    echo "BREW DRAGON_SCALE 2"; sleep 10
    echo "SPELLBOOK";           sleep 2
    cat
} | ./wizard 127.0.0.1 5000 hermione
