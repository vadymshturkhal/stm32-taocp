import asyncio
from typing import TYPE_CHECKING

from elevator_async.main import INITIAL_FLOOR, FLOORS, UNIT, CALLUP, CALLDOWN, CALLCAR
from elevator_async.decision import decision

if TYPE_CHECKING:
    from elevator_async.users import Users


class Activity:
    """
    One node of Knuth's WAIT list, expressed in asyncio terms.

    A Task alone cannot answer "is the elevator at E1?" -- it only knows that
    something is pending. So the handle carries both halves a WAIT-list node
    carries: the pending activity (`task`) and the step it resumes at
    (`nextinst`).
    """

    def __init__(self):
        self.task: asyncio.Task | None = None
        self.nextinst = None

    def start(self, coro, nextinst):
        """DELETEW + HOLDC: drop whatever was pending, schedule this instead."""
        self.cancel()
        self.nextinst = nextinst
        self.task = asyncio.create_task(coro)

    def cancel(self) -> None:
        """DELETEW. Never cancels the task we are running inside."""
        task, self.task = self.task, None
        if task is not None and not task.done() and task is not asyncio.current_task():
            task.cancel()

    @property
    def scheduled(self) -> bool:
        """Knuth's LDA 0,6 -- is this activity currently in the WAIT list?"""
        return self.task is not None and not self.task.done()


class Elevator:
    """
    ELEV1/ELEV2/ELEV3 are the WAIT list analog: one node per *activity*, not
    per entity. E3 starts the E5 and E9 timers independently of the main step
    sequence, so all three can be pending at once.

    Elevator set Users.elevator
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

        self.ELEV1 = Activity()  # elevator actions, except for E5 and E9
        self.ELEV2 = Activity()  # the independent elevator action at E5
        self.ELEV3 = Activity()  # the independent elevator action at E9

    def at(self, step) -> bool:
        """
        Is the main sequence positioned at `step`?

        Bound methods are rebuilt on every attribute access, so `is` would
        always be False here -- `==` compares function and instance.
        """
        return self.ELEV1.nextinst == step

    async def E1(self):
        # Dormant: no future event of its own, stays pending until
        # decision() supersedes it to wake the elevator back up.
        await asyncio.Event().wait()

    # [Change of state?]
    async def E2(self, delay=0):
        if delay:
            await asyncio.sleep(delay)

        # 1 STATE is GOINGUP
        if self.STATE > 0:
            # Are there calls for higher floors?
            if any(self.SHARED_STATE.CALLS[j] != 0 for j in range(self.FLOOR + 1, FLOORS)):
                    # if yes, go to E3
                    self.ELEV1.start(self.E3(), self.E3)
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
                    self.ELEV1.start(self.E3(), self.E3)
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
        self.ELEV1.start(self.E3(), self.E3)

    # [Open door]
    async def E3(self, delay=0):
        """delay is using by DECISION subroutine"""
        if delay:
            await asyncio.sleep(delay)

        # 1 Set D1 and D2 to any nonzero values
        self.D1 = 1
        self.D2 = 1

        # 2 Set E9 to start up independently after 300 units of time
        # (This activity may be cancelled in step E6 before it occurs.
        # If it has already been scheduled and not cancelled, we cancel and
        # reschedule it -- Activity.start does both.)
        self.ELEV3.start(self.E9(UNIT * 300), self.E9)

        # 3 Also set elevator activity E5 to start up independently after 76 units of time.
        # NOTE: the cancel half has no counterpart in Knuth's MIX text. There,
        # re-entering E3 naturally supersedes any still-pending E5/E4
        # continuation because the simulation is a single sequential event
        # list -- there's no way for two "instances" of an activity to be alive
        # at once. Here each activity is an independent asyncio Task, so if E3
        # fires again (e.g. U2 reopening the door) before the previous E5/E4
        # chain resolved, the old tasks would otherwise keep running alongside
        # the new ones (confirmed: this causes E6 to fire twice).
        self.ELEV2.start(self.E5(UNIT * 76), self.E5)

        # 4 Then wait 20 units of time (to simulate opening of the doors) and go to E4
        self.ELEV1.start(self.E4(UNIT * 20), self.E4)

    # [Let people out, in]
    async def E4(self, delay=0):
        if delay:
            await asyncio.sleep(delay)

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
            self.ELEV1.start(self.E4(UNIT * 25), self.E4)
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
            self.ELEV1.start(self.E4(UNIT * 25), self.E4)

        # 3 if empty
        else:
            # set D1 = 0
            self.D1 = 0

            # make D3 nonzero
            self.D3 = 1

    # [Close door]
    async def E5(self, delay=0):
        if delay:
            await asyncio.sleep(delay)

        # if D1 != 0: wait 40 units and repeat this step
        # (the doors flutter a little, but they spring open again, since someone is still getting out or in)
        if self.D1 != 0:
            self.ELEV2.start(self.E5(UNIT * 40), self.E5)
        else:  # set D3 = 0 and set elevator to start at step E6 after 20 units of time
            self.D3 = 0
            # E6 belongs to the main sequence, so it goes on ELEV1 -- not on
            # ELEV2, which this coroutine is itself running as. And it is
            # scheduled with the delay rather than after it: those 20 units are
            # exactly the window U2 means by "the doors are now closing", so
            # ELEV1 has to read as E6 throughout them.
            self.ELEV1.start(self.E6(UNIT * 20), self.E6)

    # [Prepare to move]
    async def E6(self, delay=0):
        if delay:
            await asyncio.sleep(delay)

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
        decision(self, self.E6)

        # 5
        if self.STATE == 0:
            # Go to E1
            self.ELEV1.start(self.E1(), self.E1)
            return

        # 6
        if self.D2 != 0:
            # cancel E9
            self.ELEV3.cancel()

        # 7
        if self.STATE > 0:
            # wait 15 units of time and go to E7
            self.ELEV1.start(self.E7(UNIT * 15), self.E7)
            return

        # 8
        if self.STATE < 0:
            # wait 15 units and go to E8
            self.ELEV1.start(self.E8(UNIT * 15), self.E8)
            return

    # [Go up a floor]
    async def E7(self, delay=0):
        if delay:
            await asyncio.sleep(delay)

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
            self.ELEV1.start(self.E2(UNIT * 14), self.E2)
            return

        # 3
        # otherwise repeat E7 -- one scheduled activity per floor, mirroring
        # MIX's "set NEXTINST <- E7 and reschedule" rather than an inner loop
        self.ELEV1.start(self.E7(), self.E7)

    # [Go down a floor]
    async def E8(self, delay=0):
        if delay:
            await asyncio.sleep(delay)

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
            self.ELEV1.start(self.E2(UNIT * 23), self.E2)
            return

        # 3
        # otherwise repeat E8 -- see the note in E7
        self.ELEV1.start(self.E8(), self.E8)

    # [Set inaction indicator]
    async def E9(self, delay=0):
        if delay:
            await asyncio.sleep(delay)

        # set D2 = 0 and perform the DECISION subroutine
        self.D2 = 0
        decision(self, self.E9)
