from __future__ import annotations

import asyncio
from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from elevator import Elevator

from main import STATE, FLOORS, INITIAL_FLOOR, UNIT


async def decision(elevator: Elevator, caller=None):
    # NOTE: this is async (its D2/D5 branches await asyncio.sleep(UNIT*20)
    # internally) so callers can either `await` it or fire-and-forget it via
    # asyncio.create_task(...). E6/E9 await it directly since their own next
    # steps depend on STATE having settled first. U2 fires it as an
    # independent task instead, since queuing the user in U3 has no
    # dependency on this call's outcome -- but that also means if decision()
    # ever raises from that call site, the exception is silently swallowed
    # (never retrieved) rather than propagating anywhere.
    # D1 [Decision necessary?]
    if elevator.STATE != STATE.NEUTRAL:
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
    if j == -1 and caller ==  elevator.E6:
        j = INITIAL_FLOOR

    # D4 [Set STATE]
    if elevator.FLOOR > j:
        elevator.STATE = STATE.GOINGDOWN
    elif elevator.FLOOR < j:
        elevator.STATE = STATE.GOINGUP

    # D5 [Elevator dormant]
    if elevator.is_running(elevator.E1) and j != INITIAL_FLOOR:
        # Prepare to move
        await asyncio.sleep(UNIT * 20)
        elevator.cancel(elevator.E1)
        elevator.cancel(elevator.E6)
        task = asyncio.create_task(elevator.E6())
        elevator.tasks[elevator.E6] = task
    return
