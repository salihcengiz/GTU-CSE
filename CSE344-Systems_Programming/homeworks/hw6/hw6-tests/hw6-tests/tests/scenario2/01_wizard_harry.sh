#!/usr/bin/env bash
# Scenario 2 - Terminal 2: wizard "harry".
# PDF sequence:
#   BREW MOONSTONE 10
#   SPELLBOOK              (-> MOONSTONE:10)
#   BREW UNICORN_HAIR 15
#   SPELLBOOK              (-> MOONSTONE:10,UNICORN_HAIR:15)
#   CONSUME MOONSTONE 5
#   CONSUME UNICORN_HAIR 20   (-> ERR INSUFFICIENT_INGREDIENTS: only 15 in spellbook, but global has 35; depends on global)
#   BREW BUTTERBEER 5         (-> ERR UNKNOWN_INGREDIENT: not in ingredients.txt)
#   Ctrl+C
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 2 - WIZARD harry"
feed_and_wait \
    "BREW MOONSTONE 10" \
    "SPELLBOOK" \
    "BREW UNICORN_HAIR 15" \
    "SPELLBOOK" \
    "CONSUME MOONSTONE 5" \
    "CONSUME UNICORN_HAIR 20" \
    "BREW BUTTERBEER 5" \
    | ./wizard 127.0.0.1 5000 harry
