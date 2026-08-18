#!/bin/bash
# Time elevator_c only, notrace build (elevator_sim_notrace -- trace() compiles
# to an empty function, pure simulate-and-schedule cost, no I/O in the loop).
#
# Usage:
#   ./measure_c.sh ["<space-separated user counts>"] [repeats]
# Example:
#   ./measure_c.sh "100 1000 10000 100000 1000000" 5
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ELEVATOR_DIR="$(dirname "$SCRIPT_DIR")"
ELEVATOR_C_DIR="$ELEVATOR_DIR/C_Native"

USER_COUNTS="${1:-4 66 1000 10000}"
REPEATS="${2:-5}"

echo "Building elevator_c (make -C $ELEVATOR_C_DIR elevator_sim_notrace)..."
make -C "$ELEVATOR_C_DIR" elevator_sim_notrace >/dev/null
echo

run_timed() {
    local best="" total=0 elapsed
    for ((i = 0; i < REPEATS; i++)); do
        local start end
        start=$(date +%s%N)
        "$@" > /dev/null
        end=$(date +%s%N)
        elapsed=$(( (end - start) / 1000000 ))  # ms
        total=$((total + elapsed))
        if [[ -z "$best" || $elapsed -lt $best ]]; then
            best=$elapsed
        fi
    done
    local avg=$((total / REPEATS))
    echo "min=${best}ms avg=${avg}ms"
}

printf "%-10s %-22s\n" "Users" "notrace"
printf "%-10s %-22s\n" "-----" "-----------------"
for n in $USER_COUNTS; do
    notrace_result=$(run_timed "$ELEVATOR_C_DIR/elevator_sim_notrace" "$n")
    printf "%-10s %-22s\n" "$n" "$notrace_result"
done
