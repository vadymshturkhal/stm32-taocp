from __future__ import annotations

from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from elevator import Elevator

from main import STATE, FLOORS, INITIAL_FLOOR, UNIT


def decision(elevator: Elevator, caller: str | None = None):
    # D1 [Decision necessary?]
    if elevator.STATE != STATE.NEUTRAL:
        return
    
    # D2 [Should door open?]
    if elevator.is_running("E1") and elevator.SHARED_STATE.CALLS[INITIAL_FLOOR] != 0:
        # if elevator positioned at E1: after 20 units of time start E3 and exit
        elevator.start_after(UNIT * 20, "E3")
        return

    # D3 [Any calls?]
    j = -1
    for i in range(FLOORS):
        if i == elevator.FLOOR:
            continue

        if elevator.SHARED_STATE.CALLS[i] == 0:
            continue

        j = i
        break
        
    # no such j exists and invoked by E6
    if j == -1 and caller == "E6":
        j = 2

    # D4 [Set STATE]
    if elevator.FLOOR > j:
        elevator.STATE = STATE.GOINGDOWN
    elif elevator.FLOOR < j:
        elevator.STATE = STATE.GOINGUP

    # D5 [Elevator dormant]
    if elevator.is_running("E1") and j != 2:
        # Prepare to move
        elevator.start_after(UNIT * 20, "E6")
    return
