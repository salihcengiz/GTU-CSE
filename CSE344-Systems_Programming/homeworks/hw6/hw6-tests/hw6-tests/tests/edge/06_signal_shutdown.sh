#!/usr/bin/env bash
# Edge: server Ctrl+C while clients connected.
# Each client should receive "SERVER SHUTDOWN" and exit cleanly.
# Run this in a CLIENT terminal AFTER you Ctrl+C the server.
set -u
source "$(dirname "$0")/../lib.sh"
go_root
banner "EDGE - SERVER SHUTDOWN broadcast"
echo "Connect a client and then Ctrl+C the SERVER terminal."
feed_and_wait \
    "BREW MOONSTONE 1" \
    | ./wizard 127.0.0.1 5000 shutdown_w
