#!/usr/bin/env bash
# Scenario 2 - Terminal 3: professor "mcgonagall".
# PDF sequence:
#   INSPECT DRAGON_SCALE   (-> OK INSPECT DRAGON_SCALE 50)
#   SCROLL                 (-> all ingredients)
#   ROSTER                 (-> harry,mcgonagall)
#   INSPECT BUTTERBEER     (-> ERR UNKNOWN_INGREDIENT)
#   Ctrl+C
# Note: launch this AFTER harry has connected so ROSTER shows both.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 2 - PROFESSOR mcgonagall"
feed_and_wait \
    "INSPECT DRAGON_SCALE" \
    "SCROLL" \
    "ROSTER" \
    "INSPECT BUTTERBEER" \
    | ./professor 127.0.0.1 5000 mcgonagall
