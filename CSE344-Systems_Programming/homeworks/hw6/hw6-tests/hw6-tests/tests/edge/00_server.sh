#!/usr/bin/env bash
# Shared server for edge tests (n=4, t=30s).
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "EDGE TESTS - SERVER (n=4, t=30s)"
exec ./hogwarts -p 5000 -s ingredients.txt -l server.log -n 4 -t 30
