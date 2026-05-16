#!/usr/bin/env bash
# Edge: BREW/CONSUME/INSPECT with an ingredient not in ingredients.txt.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "EDGE - ERR UNKNOWN_INGREDIENT"
feed_and_quit \
    "BREW BUTTERBEER 5" \
    "CONSUME PUMPKIN_JUICE 1" \
    | ./wizard 127.0.0.1 5000 unkown_ing_w
echo
banner "EDGE - INSPECT unknown ingredient"
feed_and_quit \
    "INSPECT PUMPKIN_JUICE" \
    | ./professor 127.0.0.1 5000 unkown_ing_p
