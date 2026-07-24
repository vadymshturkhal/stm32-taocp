from __future__ import annotations
import asyncio
from typing import TYPE_CHECKING

from main import User

if TYPE_CHECKING:
    from elevator import Elevator

# U1
# IN
# OUT
# GIVEUPTIME
# INTERTIME
# Another User = TIME + INTERTIME


class Users:
    def __init__(self, elevator: Elevator = None):
        self.elevator = elevator  # set by Elevator.__init__ if not passed here

    def immediately(self, user: User, nextinst: str) -> None:
        """Run `nextinst` (one of this class's own U-methods) for `user`
        immediately, cancelling whatever it was previously waiting on
        (Knuth's IMMED: put the user at the front of the WAIT list).

        Unlike Knuth's version, `nextinst` is only ever a transient argument
        here, never stored as a NEXTINST field on `user`: the asyncio.Task
        created below already encodes which coroutine is running, so nothing
        else needs to look up "what's next" from the user object itself."""
        self.delete_wait(user)
        coro_fn = getattr(self, nextinst)
        user.task = asyncio.create_task(coro_fn(user))

    def delete_wait(self, user: User) -> None:
        """Cancel `user`'s single pending activity, if any (Knuth's DELETEW)."""
        if user.task is not None and not user.task.done():
            user.task.cancel()
        user.task = None

    # [Enter, prepare for successor]
    async def U1(self):
        pass

    # [Signal and wait]
    async def U2(self):
        pass

    async def U3(self):
        pass

    async def U4(self):
        pass

    async def U5(self):
        pass

    async def U6(self):
        pass
