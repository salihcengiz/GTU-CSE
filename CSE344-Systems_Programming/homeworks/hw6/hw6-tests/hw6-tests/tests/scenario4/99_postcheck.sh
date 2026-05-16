#!/usr/bin/env bash
# Scenario 4 - run AFTER server Ctrl+C.
# PDF asks for:  ps aux | grep hogwarts   (no zombie/orphan processes)
#                cat server.log           (full event trace)
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "SCENARIO 4 - POST-CHECK"
echo "--- ps aux | grep hogwarts ---"
ps aux | grep hogwarts | grep -v grep || echo "(no hogwarts process running - OK)"
echo
echo "--- cat server.log ---"
cat server.log
