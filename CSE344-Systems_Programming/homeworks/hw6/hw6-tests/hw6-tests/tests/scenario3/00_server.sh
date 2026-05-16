#!/usr/bin/env bash
# Scenario 3 - Terminal 1: Server (n=3, t=60s).
# Only 3 simultaneous clients allowed.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 3 - SERVER (n=3, t=60s)"
exec ./hogwarts -p 5000 -s ingredients.txt -l server.log -n 3 -t 60
