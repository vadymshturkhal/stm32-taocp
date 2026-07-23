import asyncio

from main import STATE, INITIAL_FLOOR, FLOORS, UNIT, CALLUP, CALLDOWN, CALLCAR, D1, D2, D3
from decision import decision


class Elevator:
    def __init__(self, SHARED_STATE, STATE=STATE.NEUTRAL, D=0):
        self.SHARED_STATE = SHARED_STATE
        self.FLOOR = INITIAL_FLOOR
        self.STATE = STATE
        self.D = D
        self.home_floor = INITIAL_FLOOR
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

    # [Change of state?]
    async def E2(self):
        # Set default state to NEUTRAL
        state = STATE.NEUTRAL

        # 1 STATE is GOINGUP
        if self.STATE == STATE.GOINGUP:
            # Are there calls for higher floors?
            calls_for_higher_floors = any(self.SHARED_STATE.CALLS[j] != 0 for j in range(self.FLOOR + 1, FLOORS))

            # Have passengers in the elevator called for lower floors?
            in_elevator_calls_for_lower_floors = any(self.SHARED_STATE.CALLS[j] & CALLCAR != 0 for j in range(self.FLOOR))

            if calls_for_higher_floors:
                   # if yes, go to E3
                    self.start("E3")
                    return

            if in_elevator_calls_for_lower_floors:
                # reverse direction of STATE
                state = STATE.GOINGDOWN

        # 2 STATE is GOINGDOWN
        else:
            # Have passengers in the elevator called for higher floors?
            in_elevator_calls_for_higher_floors = any(self.SHARED_STATE.CALLS[j] & CALLCAR != 0 for j in range(self.FLOOR + 1, FLOORS))

            # Are there calls for lower floors?
            calls_for_lower_floors = any(self.SHARED_STATE.CALLS[j] != 0 for j in range(self.FLOOR))

            if calls_for_lower_floors:
                   # if yes, go to E3
                    self.start("E3")
                    return

            if in_elevator_calls_for_higher_floors:
                # reverse direction of STATE
                state = STATE.GOINGUP

        # set STATE to NEUTRAL or reversed
        self.STATE = state

        # Set all CALL variables for the current FLOOR to zero
        self.SHARED_STATE.CALLS[self.FLOOR] = 0b000

        # jump to E3
        self.start("E3")

    # [Open door]
    async def E3(self):
        pass

    async def E4(self):
        pass

    async def E5(self):
        pass

    # [Prepare to move]
    async def E6(self):
        # 1
        # CALLCAR[self.FLOOR] = 0
        # CALLCAR is the last bit of CALLS[self.FLOOR] and we need to set it zero
        self.SHARED_STATE.CALLS[self.FLOOR] &= ~CALLCAR

        # 2
        # CALLUP is the first bit of CALLS[self.FLOOR] and we need to set it zero
        if self.STATE != STATE.GOINGDOWN:
            self.SHARED_STATE.CALLS[self.FLOOR] &= ~CALLUP

        # 3
        # CALLDOWN is the middle bit of CALLS[self.FLOOR] and we need to set it zero
        if self.STATE != STATE.GOINGUP:
            self.SHARED_STATE.CALLS[self.FLOOR] &= ~CALLDOWN

        # 4
        # Perform the DECISION subroutine
        decision(self, "E6")

        # 5
        if self.STATE == STATE.NEUTRAL:
            # Go to E1
            self.start("E1")
            return

        # 6
        if self.D & D2 != 0:
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

    # [Go up a floor]
    async def E7(self):
        """
        should be: if not is_callcar_or_callup, then if not self.home_floor, then if not CALLDOWN[FLOOR}: repeat E7
        else: check calls from above, if not: continue. else wait 14 units and go to E2
        """
        while True:
            # 1 Set FLOOR += 1 and wait 51 units of time
            self.FLOOR += 1
            await asyncio.sleep(UNIT * 51)

            # 2 Conditions
            # is CALLCAR[FLOOR] == 1 or CALLUP[FLOOR] == 1
            is_callcar_or_callup = self.SHARED_STATE.CALLS[self.FLOOR] & (CALLCAR | CALLUP)

            if is_callcar_or_callup:
                should_stop = True
            else:
                # if FLOOR == home_floor or CALLDOWN[FLOOR] == 1
                at_home_or_down_call = self.FLOOR == self.home_floor or self.SHARED_STATE.CALLS[self.FLOOR] & CALLDOWN
                if not at_home_or_down_call:
                    should_stop = False
                else:
                    # and CALLS[j] == 0 for all j > FLOOR
                    no_calls_above = all(self.SHARED_STATE.CALLS[j] == 0 for j in range(self.FLOOR + 1, FLOORS))
                    should_stop = no_calls_above

            if should_stop:
                # wait 14 units (for deceleration) and go to E2
                await asyncio.sleep(UNIT * 14)
                self.start("E2")
                return

            # 3
            # otherwise repeat E7

    # [Go down a floor]
    async def E8(self):
        while True:
            # 1 Set FLOOR -= 1 and wait 61 units of time
            self.FLOOR -= 1
            await asyncio.sleep(UNIT * 61)

            # 2 Conditions
            # is CALLCAR[FLOOR] == 1 or CALLDOWN[FLOOR] == 1
            is_callcar_or_calldown = self.SHARED_STATE.CALLS[self.FLOOR] & (CALLCAR | CALLDOWN)

            if is_callcar_or_calldown:
                should_stop = True
            else:
                # if FLOOR == home_floor or CALLUP[FLOOR] == 1
                at_home_or_up_call = self.FLOOR == self.home_floor or self.SHARED_STATE.CALLS[self.FLOOR] & CALLUP
                if not at_home_or_up_call:
                    should_stop = False
                else:
                    # and CALLS[j] == 0 for all j < FLOOR
                    no_calls_below = all(self.SHARED_STATE.CALLS[j] == 0 for j in range(self.FLOOR))
                    should_stop = no_calls_below

            if should_stop:
                # wait 23 units (for deceleration) and go to E2
                await asyncio.sleep(UNIT * 23)
                self.start("E2")
                return

            # 3
            # otherwise repeat E8

    # [Set inaction indicator]
    async def E9(self):
        # set D2 = 0 and perform the DECISION subroutine
        self.D &= ~D2
        decision(self, "E9")
        return
