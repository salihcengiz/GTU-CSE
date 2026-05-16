#!/usr/bin/env bash
# Edge: split a single command across multiple TCP writes -> server's line
# buffer must reassemble. We use a temp FIFO so nc reads byte-by-byte from us.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "EDGE - PARTIAL READ (one command split into many TCP segments)"

(
    sleep 0.3
    # send ENROLL byte-by-byte
    line="ENROLL WIZARD partial"
    for (( i=0; i<${#line}; i++ )); do
        printf '%s' "${line:$i:1}"
        sleep 0.05
    done
    printf '\n'
    sleep 0.5
    # now send BREW MOONSTONE 7 in two chunks
    printf 'BREW MOONS'
    sleep 0.5
    printf 'TONE 7\n'
    sleep 0.5
    printf 'SPELLBOOK\n'
    sleep 0.5
    printf 'APPARATE\n'
    sleep 0.5
) | nc 127.0.0.1 5000
