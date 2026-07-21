from __future__ import annotations

from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from elevator import Elevator

from main import STATE, FLOORS, INITIAL_FLOOR, UNIT


def decision(elevator: Elevator, caller: str | None = None):
    # D1
    if elevator.STATE != STATE.NEUTRAL:
        return
    
    # D2
    # if elevator positioned at E1
    if (elevator.is_running("E1") 
            and elevator.CALLUP[INITIAL_FLOOR] != 0 
            and elevator.CALLCAR[INITIAL_FLOOR] != 0 
            and elevator.CALLDOWN[INITIAL_FLOOR] != 0):
 
        # after 20 units of time start E3 and exit
        elevator.start_after(UNIT * 20, "E3")
        return

    # D3 [Any calls?]
    j = 0b1000
    for i in range(FLOORS):
        if i == elevator.FLOOR:
            continue

        if elevator.SHARED_STATE.CALLS[i] == 0:
            continue

        if j < elevator.SHARED_STATE.CALLS[i]:
            j = elevator.SHARED_STATE.CALLS[i]
        
    # no such j exists and invoked by E6
    if j == 0b1000 and caller == "E6":
        j = 2

    return
