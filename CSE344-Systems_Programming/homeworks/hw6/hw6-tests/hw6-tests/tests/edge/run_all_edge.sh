#!/usr/bin/env bash
# Run every edge test back-to-back against a freshly-started server.
# This is an AUTOMATED smoke test - the server is started internally,
# killed at the end, and the log is dumped to stdout.
set -u
source "$(dirname "$0")/../lib.sh"
go_root

banner "EDGE BATCH - start server"
rm -f server.log
./hogwarts -p 5000 -s ingredients.txt -l server.log -n 4 -t 30 &
SP=$!
trap 'kill -INT $SP 2>/dev/null; wait $SP 2>/dev/null' EXIT
sleep 0.5

banner "01 TOOLONG"
./tests/edge/01_toolong.sh || true
sleep 0.5

banner "02 PARTIAL READ"
./tests/edge/02_partial_read.sh || true
sleep 0.5

banner "03 UNKNOWN INGREDIENT"
./tests/edge/03_unknown_ingredient.sh || true
sleep 0.5

banner "04 INSUFFICIENT"
./tests/edge/04_insufficient.sh || true
sleep 0.5

banner "05 HANGUP + slot recovery"
./tests/edge/05_hangup.sh || true
sleep 0.5

banner "08 normal APPARATE"
./tests/edge/08_apparate_normal.sh || true
sleep 0.5

banner "STOP SERVER (SIGINT)"
kill -INT $SP
wait $SP 2>/dev/null
trap - EXIT

echo
banner "FINAL SERVER LOG"
cat server.log
