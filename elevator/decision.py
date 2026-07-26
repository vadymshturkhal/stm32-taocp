from __future__ import annotations

import asyncio
from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from elevator import Elevator

from main import GOINGUP, GOINGDOWN, NEUTRAL, FLOORS, INITIAL_FLOOR, UNIT


async def decision(elevator: Elevator, caller=None):
    # D1 [Decision necessary?]
    if elevator.STATE != 0:
        return

    # D2 [Should door open?]
    if elevator.is_running(elevator.E1) and elevator.SHARED_STATE.CALLS[INITIAL_FLOOR] != 0:
        # if elevator positioned at E1: after 20 units of time start E3 and exit
        await asyncio.sleep(UNIT * 20)
        elevator.cancel(elevator.E1)
        elevator.cancel(elevator.E3)
        task = asyncio.create_task(elevator.E3())
        elevator.tasks[elevator.E3] = task
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
    if j == -1 and caller == elevator.E6:
        j = INITIAL_FLOOR

    # D4 [Set STATE]
    elevator.STATE = j - elevator.FLOOR

    # D5 [Elevator dormant]
    if elevator.is_running(elevator.E1) and j != INITIAL_FLOOR:
        # Prepare to move
        await asyncio.sleep(UNIT * 20)
        elevator.cancel(elevator.E1)
        elevator.cancel(elevator.E6)
        task = asyncio.create_task(elevator.E6())
        elevator.tasks[elevator.E6] = task
