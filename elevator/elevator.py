import asyncio
import inspect
from typing import TYPE_CHECKING

from main import INITIAL_FLOOR, FLOORS, UNIT, CALLUP, CALLDOWN, CALLCAR
from decision import decision

if TYPE_CHECKING:
    from users import Users

# FIXME: tasks and cancel
class Elevator:
    """
    Uses tasks dict as WAIT list analog for fast insert-remove nodes
    """
    def __init__(self, SHARED_STATE, USERS: "Users", STATE=0):
        self.SHARED_STATE = SHARED_STATE
        self.USERS = USERS
        USERS.elevator = self
        self.FLOOR = INITIAL_FLOOR
        self.STATE = STATE
        self.D1 = 0  # a variable that is zero except during the time people are getting in or out of the elevator
        self.D2 = 0  # a variable that becomes zero if the elevator has sat on one floor without moving for 30 sec or more
        self.D3 = 0  # a variable that is zero except when the doors are open but nobody is getting in or out of the elevator
        self.home_floor = INITIAL_FLOOR
        self.tasks: dict[str, asyncio.Task] = {}

    def is_running(self, name: str) -> bool:
        task = self.tasks.get(name)
        if task is None:
            return False
        state = inspect.getcoroutinestate(task.get_coro())
        return state != inspect.CORO_CLOSED
    
    def cancel(self, method) -> None:
        """Cancel the coroutine named `name`, if currently running."""
        task = self.tasks.get(method)
        if task is not None and not task.done():
            task.cancel()

    async def E1(self):
        # Dormant: no future event of its own, stays pending until
        # decision() cancels it to wake the elevator back up.
        await asyncio.Event().wait()

    # [Change of state?]
    async def E2(self):
        # 1 STATE is GOINGUP
        if self.STATE > 0:
            # Are there calls for higher floors?
            if any(self.SHARED_STATE.CALLS[j] != 0 for j in range(self.FLOOR + 1, FLOORS)):
                    # if yes, go to E3
                    task = asyncio.create_task(self.E3())
                    self.tasks[self.E3] = task
                    return

            # Have passengers in the elevator called for lower floors?
            if any(self.SHARED_STATE.CALLS[j] & CALLCAR != 0 for j in range(self.FLOOR)):
                # reverse direction of STATE
                self.STATE = -self.STATE
            else:
                self.STATE = 0

        # 2 STATE is GOINGDOWN
        else:
            # Are there calls for lower floors?
            if any(self.SHARED_STATE.CALLS[j] != 0 for j in range(self.FLOOR)):
                   # if yes, go to E3
                    task = asyncio.create_task(self.E3())
                    self.tasks[self.E3] = task
                    return

            # Have passengers in the elevator called for higher floors?
            if any(self.SHARED_STATE.CALLS[j] & CALLCAR != 0 for j in range(self.FLOOR + 1, FLOORS)):
                # reverse direction of STATE
                self.STATE = -self.STATE
            else:
                self.STATE = 0
    
        # Set all CALL variables for the current FLOOR to zero
        self.SHARED_STATE.CALLS[self.FLOOR] = 0b000

        # jump to E3
        task = asyncio.create_task(self.E3())
        self.tasks[self.E3] = task

    # [Open door]
    async def E3(self):
        # 1 Set D1 and D2 to any nonzero values
        self.D1 = 1
        self.D2 = 1

        # 2 Set E9 to start up independently after 300 units of time
        # (This activity may be cancelled in step E6 before it occurs. 
        # If it has already been scheduled and not cancelled, we cancel and reschedule it.)
        self.cancel(self.E9)
        task = asyncio.create_task(self.E9(UNIT * 300))
        self.tasks[self.E9] = task

        # 3 Also set elevator activity E5 to start up independently after 76 units of time.
        # NOTE: this cancel-before-reschedule guard has no counterpart in Knuth's
        # original MIX text. There, re-entering E3 naturally supersedes any
        # still-pending E5/E4 continuation because the simulation is a single
        # sequential event list -- there's no way for two "instances" of an
        # activity to be alive at once. Here, each activity is an independent
        # asyncio Task, so if E3 fires again (e.g. U2 reopening the door)
        # before the previous E5/E4 chain resolved, the old tasks would
        # otherwise keep running alongside the new ones (confirmed: this
        # causes E6 to fire twice).
        self.cancel(self.E5)
        task = asyncio.create_task(self.E5(UNIT * 76))
        self.tasks[self.E5] = task

        # 4 Then wait 20 units of time (to simulate opening of the doors) and go to E4
        await asyncio.sleep(UNIT * 20)
        self.cancel(self.E4)
        task = asyncio.create_task(self.E4())
        self.tasks[self.E4] = task

    # [Let people out, in]
    async def E4(self):
        # 1 if anyone in the ELEVATOR list has OUT == FLOOR
        elevator_list = self.SHARED_STATE.ELEVATOR
        node = elevator_list.head.left
        found_user = None
        while node is not elevator_list.head:
            if node.info.OUT == self.FLOOR:
                found_user = node.info
                break

            node = node.left

        # send the user of this type who has most recently entered, immediately to U6
        if found_user is not None:
            # cancel U4
            self.USERS.delete_user_task(found_user)
            found_user.task = asyncio.create_task(self.USERS.U6(found_user))

            # wait 25 units, and repeat E4
            await asyncio.sleep(UNIT * 25)
            task = asyncio.create_task(self.E4())
            self.tasks[self.E4] = task
            return

        # 2 if no such user exist, but QUEUE[FLOOR] is not empty
        if self.SHARED_STATE.QUEUE[self.FLOOR].size != 0:
            # send the front person immediately to U5 instead of U4
            front_person = self.SHARED_STATE.QUEUE[self.FLOOR].head.right.info

            # cancel U4
            self.USERS.delete_user_task(front_person)
            # send to U5
            front_person.task = asyncio.create_task(self.USERS.U5(front_person))

            # wait 25 units, and repeat E4
            await asyncio.sleep(UNIT * 25)
            task = asyncio.create_task(self.E4())
            self.tasks[self.E4] = task

        # 3 if empty
        else:
            # set D1 = 0
            self.D1 = 0

            # make D3 nonzero
            self.D3 = 1

            # No explicit yield needed here (Knuth's JMP CYCLE, "return to
            # simulate other events"): once this coroutine returns, its Task
            # completes and control goes back to the event loop on its own
            
            # await asyncio.sleep(0)

    # [Close door]
    async def E5(self, delay=0):
        if delay:
            await asyncio.sleep(delay)

        # if D1 != 0: wait 40 units and repeat this step
        # (the doors flutter a little, but they spring open again, since someone is still getting out or in)
        if self.D1 != 0:
            await asyncio.sleep(UNIT * 40)
            task = asyncio.create_task(self.E5())
            self.tasks[self.E5] = task
        else:  # set D3 = 0 and set elevator to start at step E6 after 20 units of time
            self.D3 = 0
            await asyncio.sleep(UNIT * 20)
            task = asyncio.create_task(self.E6())
            self.tasks[self.E6] = task

    # [Prepare to move]
    async def E6(self):
        # 1
        # CALLCAR[self.FLOOR] = 0
        # CALLCAR is the last bit of CALLS[self.FLOOR] and we need to set it zero
        self.SHARED_STATE.CALLS[self.FLOOR] &= ~CALLCAR

        # 2
        # CALLUP is the first bit of CALLS[self.FLOOR] and we need to set it zero
        if self.STATE >= 0:
            self.SHARED_STATE.CALLS[self.FLOOR] &= ~CALLUP

        # 3
        # CALLDOWN is the middle bit of CALLS[self.FLOOR] and we need to set it zero
        if self.STATE <= 0:
            self.SHARED_STATE.CALLS[self.FLOOR] &= ~CALLDOWN

        # 4
        # Perform the DECISION subroutine
        await decision(self, self.E6)

        # 5
        if self.STATE == 0:
            # Go to E1
            self.cancel(self.E1)
            task = asyncio.create_task(self.E1())
            self.tasks[self.E1] = task
            return

        # 6
        if self.D2 != 0:
            # cancel E9
            self.cancel(self.E9)

        # 7
        if self.STATE > 0:
            # wait 15 units of time and go to E7
            await asyncio.sleep(UNIT * 15)
            task = asyncio.create_task(self.E7())
            self.tasks[self.E7] = task
            return

        # 8
        if self.STATE < 0:
            # wait 15 units and go to E8
            await asyncio.sleep(UNIT * 15)
            task = asyncio.create_task(self.E8())
            self.tasks[self.E8] = task
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
                task = asyncio.create_task(self.E2())
                self.tasks[self.E2] = task
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
                task = asyncio.create_task(self.E2())
                self.tasks[self.E2] = task
                return

            # 3
            # otherwise repeat E8

    # [Set inaction indicator]
    async def E9(self, delay=0):
        if delay:
            await asyncio.sleep(delay)

        # set D2 = 0 and perform the DECISION subroutine
        self.D2 = 0
        await decision(self, self.E9)
