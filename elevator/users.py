from __future__ import annotations
import asyncio
import random
from typing import Callable, TYPE_CHECKING

from main import User, FLOORS, UNIT, CALLUP, CALLDOWN, CALLCAR
from decision import decision

if TYPE_CHECKING:
    from elevator import Elevator

# U1
# IN
# OUT
# GIVEUPTIME
# INTERTIME
# Another User = TIME + INTERTIME


class Users:
    def __init__(self, elevator: Elevator = None, next_user_params: Callable = None):
        self.elevator = elevator  # set by Elevator.__init__ if not passed here

        # Callable returning (IN, OUT, GIVEUPTIME, INTERTIME) for the next
        # arrival. Knuth leaves the actual distribution "determined in some
        # manner that will not be specified here", so this is injectable:
        # tests can pass a fixed/fake generator instead of depending on
        # real randomness, and the default below is only a placeholder
        # guess, not something derived from the book.
        self.next_user_params = next_user_params or self._default_next_user_params

    def _default_next_user_params(self):
        IN = random.randint(0, FLOORS - 1)
        OUT = random.choice([floor for floor in range(FLOORS) if floor != IN])
        GIVEUPTIME = random.uniform(50, 150) * UNIT
        INTERTIME = random.expovariate(1 / (100 * UNIT))
        return IN, OUT, GIVEUPTIME, INTERTIME

    def delete_user_task(self, user: User) -> None:
        """Cancel `user`'s single pending activity, if any (Knuth's DELETEW)."""
        if user.task is not None and not user.task.done():
            user.task.cancel()
        user.task = None

    # [Enter, prepare for successor]
    async def U1(self):
        IN, OUT, GIVEUPTIME, INTERTIME = self.next_user_params()
        user = User(self.elevator.SHARED_STATE, IN, OUT, GIVEUPTIME)
        user.arrival_time = self.elevator.SHARED_STATE.TIME

        # Schedule the next arrival independently. Unlike Elevator's
        # E-steps, there's only ever one of these pending at a time, so no
        # cancel-before-reschedule bookkeeping is needed here.
        async def _next_arrival():
            await asyncio.sleep(INTERTIME)
            await self.U1()

        asyncio.create_task(_next_arrival())

        # This user proceeds to U2 immediately
        self.delete_user_task(user)
        user.task = asyncio.create_task(self.U2(user))

    # [Signal and wait]
    async def U2(self, user: User):
        """
        The purpose of this step is to call for the elevator;
        some special cases arise if the elevator is already on the right floor.
        """
        elevator = self.elevator

        # 1 
        # if FLOOR == IN
        if elevator.FLOOR == user.IN:
            # 2 
            # and if the elevator's next action is step E6 
            # (if the elevator doors are now closing): 
            if elevator.is_running(elevator.E6):
                # send the elevator immediately to E3 and cancel its activity E6
                # (this means that the doors will open again before the elevator moves)
                elevator.cancel(elevator.E6)
                elevator.cancel(elevator.E3)
                task = asyncio.create_task(elevator.E3())
                elevator.tasks[elevator.E3] = task
                return

            # 3 
            # and if D3 != 0
            if elevator.D3 != 0:
                # set D3 = 0, D1 to a nonzero value and start up the elevator's activity E4 again
                elevator.D3 = 0
                elevator.D1 = 1

                elevator.cancel(elevator.E4)
                task = asyncio.create_task(elevator.E4())
                elevator.tasks[elevator.E4] = task
                return

        # 4 
        # in all other cases the user sets CALLUP[IN] = 1 or CALLDOWN[IN] = 1
        # according as OUT > IN or OUT < IN
        if user.OUT > user.IN:
            elevator.SHARED_STATE.CALLS[user.IN] |= CALLUP
        else:
            elevator.SHARED_STATE.CALLS[user.IN] |= CALLDOWN

        # 5 
        # and if D2 == 0 or the elevator in its "dormant" position E1, the DESICION performed
        if elevator.D2 == 0 or elevator.is_running(elevator.E1):
            asyncio.create_task(decision(elevator))

        self.delete_user_task(user)
        user.task = asyncio.create_task(self.U3(user))

    # [Enter queue]
    async def U3(self, user: User):
        # 1 
        # insert this user at the rear of QUEUE[IN]
        self.elevator.SHARED_STATE.QUEUE[user.IN].insert_right(user)

        # 2
        # now the user waits patiently for GIVEUPTIME units of time, unless the elevator arrives first
        # (unless step E4 of the elevator routine sends this user to U5 and cancels the scheduled activity U4)
        self.delete_user_task(user)
        user.task = asyncio.create_task(self.U4(user))

    # [Give up]
    async def U4(self, user: User):
        pass

    async def U5(self, user: User):
        pass

    async def U6(self, user: User):
        pass
