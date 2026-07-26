import asyncio
import sys
import os
import time
import random

# import DoublyLinkedList and STORAGE_POOL
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "data_structures"))
from doubly_linked_list import DoublyLinkedList, DoublyNode
from storage_pool import STORAGE_POOL


"""
User


FLOORS: 0, 1, 2, 3, 4

On each floor there are two call buttons, one for UP and one for DOWN.
(Actually floor 0 has only UP and floor 4 has only DOWN, but we may ignore that anomaly since the excess buttons will never be used.)

Corresponding to these buttons, there are ten variables CALLUP[j] and CALLDOWN[j], 0 <= j <= 4.

There are also variables CALLCAR[j], 0 <= j <= 4, representing buttons within the elevator car, which direct it to a destination floor.
When a person presses a button, the appropriate variable is set to 1; the elevator clears the variable to 0 after the request has been fulfilled.


Elevator:
The elevator takes a certain amount of time to open and close its doors, to accelerate and decelerate, and to get from one floor to another.
Elevator STATES are: NEUTRAL is 0, GOINGUP is positive value, GOINGDOWN is negative value
"""

FLOORS = 5
INITIAL_FLOOR = 2
UNIT = 0.0001

# Masks
CALLUP = 0b100
CALLDOWN = 0b010
CALLCAR = 0b001

class User:
    def __init__(self, SHARED_STATE, IN, OUT, GIVEUPTIME):
        self.SHARED_STATE = SHARED_STATE
        self.IN = IN
        self.OUT = OUT
        self.GIVEUPTIME = GIVEUPTIME
        self.position_node = None   # this user's node in ELEVATOR or QUEUE[floor]
        self.task = None            # this user's single pending asyncio.Task, if any
        self.arrival_time = None    # SHARED_STATE.TIME when this user entered the system

def generate_users(shared_state, MAX_USERS: int) -> list:
    users = []
    for _ in range(MAX_USERS):
        IN = random.randint(0, FLOORS - 1)
        OUT = random.choice([floor for floor in range(FLOORS) if floor != IN])
        GIVEUPTIME = random.uniform(50, 150) * UNIT
        users.append(User(shared_state, IN, OUT, GIVEUPTIME))
    return users


class SharedState:
    def __init__(self, MAX_USERS: int):
        self._start_time = time.monotonic()
        self.CALLS = [0b000] * FLOORS

        # Shared storage pool: one AVAIL list serving ELEVATOR and every QUEUE[floor],
        # per Knuth's convention of pooling nodes across related linked structures.
        # Each User occupies one node at a time, in either ELEVATOR or QUEUE[floor].
        # +1 sentinel head node per list (ELEVATOR, plus one per floor's queue).
        self.storage_pool = STORAGE_POOL(node_class=DoublyNode, size=MAX_USERS + 1 + FLOORS)

        self.ELEVATOR = DoublyLinkedList(self.storage_pool)                        # Users currently riding the car
        self.QUEUE = [DoublyLinkedList(self.storage_pool) for _ in range(FLOORS)]  # Users waiting at each floor

    @property
    def TIME(self):
        """Current simulated time, in the same units UNIT-scaled delays use.

        Unlike Knuth's WAIT-list dispatcher, which advances a stored clock
        by hand each time it pops the next event, we're running on real
        asyncio concurrency with potentially many sleeps in flight at once
        -- there's no single "next event" to advance TIME to. So TIME is
        just derived from real elapsed wall-clock time instead of being
        accumulated: it's always correct regardless of how many concurrent
        activities are pending, and asyncio.sleep(delay) already IS "run at
        TIME + delay" implicitly, with no separate computation needed.
        """
        return (time.monotonic() - self._start_time) / UNIT


if __name__ == "__main__":
    from users import Users
    from elevator import Elevator

    MAX_USERS = 2
    shared_state = SharedState(MAX_USERS)
    users_list = generate_users(shared_state, MAX_USERS)

    users = Users(users=users_list)
    elevator = Elevator(shared_state, users)

    async def run_simulation(elevator, users):
        elevator.tasks[elevator.E1] = asyncio.create_task(elevator.E1())
        await users.U1()

        while not (elevator.is_running(elevator.E1)
                and elevator.SHARED_STATE.ELEVATOR.size == 0
                and all(q.size == 0 for q in elevator.SHARED_STATE.QUEUE)):
            await asyncio.sleep(UNIT)

    asyncio.run(run_simulation(elevator, users))
