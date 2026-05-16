#!/usr/bin/env bash
# Scenario 4 - Terminal 1: Server (n=6, t=10s).
# Short timeout: idle clients get TIMEOUT DISCONNECT after 10s.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 4 - SERVER (n=6, t=10s)"
exec ./hogwarts -p 5000 -s ingredients.txt -l server.log -n 6 -t 10
