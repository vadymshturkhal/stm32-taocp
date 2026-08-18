#!/bin/bash
# Time comparison: elevator_c (native, gcc -O3 -flto) vs elevator_wait_list (Python),
# both running WITHOUT trace output -- pure simulate-and-schedule cost, no
# printf()/print() formatting or I/O in the loop.
#
# C:      elevator_sim_notrace, built without -DTRACE (trace() compiles to an empty function).
# Python: TRACE=0 main.py (print() monkey-patched to a no-op; see elevator_wait_list/main.py).
#
# Usage:
#   ./compare_c_python_no_trace.sh ["<space-separated user counts>"] [repeats]
# Example:
#   ./compare_c_python_no_trace.sh "4 66 1000 10000" 5
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ELEVATOR_DIR="$(dirname "$SCRIPT_DIR")"
ELEVATOR_C_DIR="$ELEVATOR_DIR/C_Native"
ELEVATOR_PY_DIR="$ELEVATOR_DIR/Python_WAIT_LIST"

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

printf "%-10s %-22s %-22s\n" "Users" "C (notrace)" "Python (TRACE=0)"
printf "%-10s %-22s %-22s\n" "-----" "-----------------" "-----------------"
for n in $USER_COUNTS; do
    c_result=$(run_timed "$ELEVATOR_C_DIR/elevator_sim_notrace" "$n")
    py_result=$(TRACE=0 run_timed python3 "$ELEVATOR_PY_DIR/main.py" "$n")
    printf "%-10s %-22s %-22s\n" "$n" "$c_result" "$py_result"
done
