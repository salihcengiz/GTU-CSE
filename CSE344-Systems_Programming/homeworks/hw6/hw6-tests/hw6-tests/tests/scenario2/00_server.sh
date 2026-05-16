#!/usr/bin/env bash
# Scenario 2 - Terminal 1: Server (n=4, t=30s)
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 2 - SERVER (n=4, t=30s)"
exec ./hogwarts -p 5000 -s ingredients.txt -l server.log -n 4 -t 30
