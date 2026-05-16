#!/usr/bin/env bash
# Edge: line longer than 512 bytes -> server sends ERR TOOLONG.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "EDGE - ERR TOOLONG (line > 512 bytes)"
{
    sleep 0.3
    echo "ENROLL WIZARD lineboi"; sleep 0.5
    # 600 'A's followed by newline -> exceeds the 512-byte cap
    head -c 600 < /dev/zero | tr '\0' 'A'
    echo
    sleep 0.5
    echo "SPELLBOOK"            # normal command after recovery
    sleep 0.5
    echo "APPARATE"
    sleep 0.5
} | nc 127.0.0.1 5000
