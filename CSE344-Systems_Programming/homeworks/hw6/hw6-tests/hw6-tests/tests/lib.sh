# Common helpers for test scripts. Sourced, not executed.

# Go to project root regardless of where script lives.
go_root() {
    local d
    d="$(cd "$(dirname "${BASH_SOURCE[1]}")/../.." && pwd)"
    cd "$d"
}

# Default inter-command sleep (seconds). Override with SLEEP=...
: "${SLEEP:=1}"

# Send a sequence of lines to stdout of the pipe, then keep stdin open with cat.
# Each argument is one command line.
# Usage:   feed_and_wait "CMD1" "CMD2" ... | ./wizard ...
feed_and_wait() {
    local cmd
    for cmd in "$@"; do
        printf '%s\n' "$cmd"
        sleep "$SLEEP"
    done
    # keep stdin alive so the client doesn't auto-exit; user presses Ctrl+C
    cat
}

# Send a sequence then send APPARATE and exit (no Ctrl+C needed).
feed_and_quit() {
    local cmd
    for cmd in "$@"; do
        printf '%s\n' "$cmd"
        sleep "$SLEEP"
    done
    printf 'APPARATE\n'
    sleep 0.5
}

# Banner for clarity in screenshots.
banner() {
    echo
    echo "========================================================"
    echo "  $*"
    echo "========================================================"
    echo
}
