#!/usr/bin/env bash
# Edge: two ENROLLs with same username across types -> second gets ERR ENROLL name_taken.
# Open one client first via terminal A:  ./tests/edge/07_name_taken.sh first
# Then in terminal B run:                ./tests/edge/07_name_taken.sh second
set -u
source "$(dirname "$0")/../lib.sh"
go_root

case "${1:-first}" in
    first)
        banner "EDGE - name_taken: FIRST wizard enrolling as 'severus'"
        feed_and_wait \
            | ./wizard 127.0.0.1 5000 severus
        ;;
    second)
        banner "EDGE - name_taken: SECOND client (professor!) tries 'severus'"
        feed_and_wait \
            | ./professor 127.0.0.1 5000 severus
        ;;
    *)
        echo "Usage: $0 [first|second]" >&2
        exit 1
        ;;
esac
