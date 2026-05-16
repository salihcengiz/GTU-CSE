#!/usr/bin/env bash
# Edge: CONSUME more than the current global stock -> ERR INSUFFICIENT_INGREDIENTS.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "EDGE - ERR INSUFFICIENT_INGREDIENTS"
feed_and_quit \
    "CONSUME UNICORN_HAIR 1000" \
    "INSPECT UNICORN_HAIR" \
    | ./wizard 127.0.0.1 5000 insuff_w
