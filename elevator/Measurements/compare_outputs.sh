#!/bin/bash
# Correctness check: diff elevator_c's traced output against elevator_wait_list's.
# Both use values_seed(1), so for the same user count they should produce
# byte-identical trace lines if the ports agree.
#
# Usage:
#   ./compare_outputs.sh [user_count]
# Example:
#   ./compare_outputs.sh 66
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ELEVATOR_DIR="$(dirname "$SCRIPT_DIR")"
ELEVATOR_C_DIR="$ELEVATOR_DIR/C_Native"
ELEVATOR_PY_DIR="$ELEVATOR_DIR/Python_WAIT_LIST"

USERS="${1:-66}"
C_OUT="$SCRIPT_DIR/elevator_output_c.txt"
PY_OUT="$SCRIPT_DIR/elevator_output_py.txt"

echo "Building elevator_c (make -C $ELEVATOR_C_DIR elevator_sim)..."
make -C "$ELEVATOR_C_DIR" elevator_sim >/dev/null
echo

echo "Running elevator_sim $USERS > $C_OUT"
"$ELEVATOR_C_DIR/elevator_sim" "$USERS" > "$C_OUT"

echo "Running main.py $USERS > $PY_OUT"
python3 "$ELEVATOR_PY_DIR/main.py" "$USERS" > "$PY_OUT"

echo
if diff -u "$C_OUT" "$PY_OUT"; then
    echo "MATCH: C and Python trace output are identical for $USERS users."
else
    echo "MISMATCH: see diff above ('-' = C only, '+' = Python only)."
    exit 1
fi
