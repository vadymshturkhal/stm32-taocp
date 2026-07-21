from enum import Enum
import asyncio

from main import STATE, FLOORS, INITIAL_FLOOR, UNIT
from decision import decision


class Elevator:
    def __init__(self, SHARED_STATE, FLOOR=2, STATE=STATE.NEUTRAL, D1=0, D2=0, D3=0):
        self.SHARED_STATE = SHARED_STATE
        self.FLOOR = FLOOR
        self.STATE = STATE
        self.D1 = D1
        self.D2 = D2
        self.D3 = D3
        self.home_floor = 2
        self.CALLCAR = [0] * FLOORS
        self.CALLUP = [0] * FLOORS
        self.CALLDOWN = [0] * FLOORS
        self.tasks: dict[str, asyncio.Task] = {}

    def is_running(self, name: str) -> bool:
        """True if the coroutine named `name` (e.g. "E1") is currently active."""
        task = self.tasks.get(name)
        return task is not None and not task.done()
    
    def start(self, name: str) -> asyncio.Task:
        """Start the coroutine named `name` as a task, if not already running."""
        if self.is_running(name):
            return self.tasks[name]
        coro_fn = getattr(self, name)
        task = asyncio.create_task(coro_fn())
        self.tasks[name] = task
        return task
    
    def cancel(self, name: str) -> None:
        """Cancel the coroutine named `name`, if currently running."""
        task = self.tasks.get(name)
        if task is not None and not task.done():
            task.cancel()

    def start_after(self, delay: float, name: str) -> asyncio.Task:
        """Schedule start(name) to run after `delay`, without blocking the
        caller. Tracked under "<name>_timer" so the pending delay itself
        can be cancelled too, e.g. if conditions change before it elapses."""
        async def _delayed():
            await asyncio.sleep(delay)
            self.start(name)

        task = asyncio.create_task(_delayed())
        self.tasks[f"{name}_timer"] = task
        return task

    async def E1(self):
        await asyncio.sleep(0)

    # [Prepare to move]
    async def E6(self):
        # 1
        # CALLCAR[self.FLOOR] = 0
        # CALLCAR is the last bit of CALLS[self.FLOOR] and we need to set it zero
        self.SHARED_STATE.CALLS[self.FLOOR] &= 0b110

        # 2
        # CALLUP is the first bit of CALLS[self.FLOOR] and we need to set it zero
        if self.STATE != STATE.GOINGDOWN:
            self.SHARED_STATE.CALLS[self.FLOOR] &= 0b011

        # 3
        # CALLDOWN is the middle bit of CALLS[self.FLOOR] and we need to set it zero
        if self.STATE != STATE.GOINGUP:
            self.SHARED_STATE.CALLS[self.FLOOR] &= 0b101

        # 4
        # Perform the DECISION subroutine
        decision(self, "E6")

        # 5
        if self.STATE == STATE.NEUTRAL:
            # Go to E1
            self.start("E1")
            return

        # 6
        if self.D2 != 0:
            # cancel E9
            self.cancel("E9")
            return

        # 7 
        if self.STATE == STATE.GOINGUP:
            # wait 15 units of time and go to E7
            self.start_after(UNIT * 15, "E7")
            return

        # 8
        if self.STATE == STATE.GOINGDOWN:
            # wait 15 units and go to E8
            self.start_after(UNIT * 15, "E8")
            return

    async def E7(self):
        pass

    async def E8(self):
        pass

    # [Set inaction indicator]
    async def E9(self):
        # set D2 = 0 and perform the DECISION subroutine
        self.D2 = 0
        decision(self, "E9")
        return


if __name__ == "__main__":
    elevator = Elevator()
    print(elevator.CALLCAR)
