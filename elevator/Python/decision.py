from __future__ import annotations

import asyncio
from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from elevator import Elevator

from main import FLOORS, INITIAL_FLOOR, UNIT


def decision(elevator: Elevator, caller=None):
    # D1 [Decision necessary?]
    if elevator.STATE != 0:
        return

    # D2 [Should door open?]
    if elevator.is_running(elevator.E1) and elevator.SHARED_STATE.CALLS[INITIAL_FLOOR] != 0:
        # if elevator positioned at E1: after 20 units of time start E3 and exit
        elevator.cancel(elevator.E1)
        elevator.cancel(elevator.E3)
        task = asyncio.create_task(elevator.E3(UNIT * 20))
        elevator.tasks[elevator.E3] = task
        return

    # D3 [Any calls?]
    # 1 
    # find the smallest j != FLOOR for which CALLUP[j], CALLCAR[j] or CALLDOWN[j] is nonzero
    # and go on to step D4
    j = -1
    for i in range(FLOORS):
        if i == elevator.FLOOR:
            continue

        if elevator.SHARED_STATE.CALLS[i] == 0:
            continue

        j = i
        break

    # 2
    # but if no such j exists
    if j == -1:
        # otherwise exit from this subroutine
        if caller != elevator.E6:
            return

        # then set j = INITIAL_FLOOR if the DECISION subroutine is currently being invoked by step E6
        j = INITIAL_FLOOR

    # D4 [Set STATE]
    elevator.STATE = j - elevator.FLOOR

    # D5 [Elevator dormant]
    if elevator.is_running(elevator.E1) and elevator.STATE != 0:
        # Prepare to move
        elevator.cancel(elevator.E1)
        elevator.cancel(elevator.E6)
        task = asyncio.create_task(elevator.E6(UNIT * 20))
        elevator.tasks[elevator.E6] = task
