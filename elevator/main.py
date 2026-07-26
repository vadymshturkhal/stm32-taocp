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
    def __init__(self, SHARED_STATE, IN, OUT, GIVEUPTIME, NAME=None):
        self.SHARED_STATE = SHARED_STATE
        self.IN = IN
        self.OUT = OUT
        self.GIVEUPTIME = GIVEUPTIME
        self.position_node = None   # this user's node in ELEVATOR or QUEUE[floor]
        self.task = None            # this user's single pending asyncio.Task, if any
        self.arrival_time = None    # SHARED_STATE.TIME when this user entered the system
        self.NAME = NAME

def generate_users(shared_state, MAX_USERS: int) -> list:
    users = []
    for i in range(MAX_USERS):
        IN = random.randint(0, FLOORS - 1)
        OUT = random.choice([floor for floor in range(FLOORS) if floor != IN])
        GIVEUPTIME = random.uniform(100, 600) * UNIT
        NAME = f"User{i}"
        users.append(User(shared_state, IN, OUT, GIVEUPTIME, NAME))
    return users


class SharedState:
    def __init__(self, MAX_USERS: int = 100):
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


def state_letter(state: int) -> str:
    if state > 0:
        return "U"
    if state < 0:
        return "D"
    return "N"


def flag(value) -> str:
    return "X" if value else "0"


ACTIONS = {
    "E1": "Elevator dormant",
    "E2": "Elevator stops",
    "E3": "Elevator doors start to open",
    "E4": "Elevator lets people in/out",
    "E5": "Elevator doors start to close",
    "E6": "Elevator prepares to move",
    "E7": "Elevator moving up",
    "E8": "Elevator moving down",
    "U2": "User signals and waits",
    "U3": "User enters queue",
    "U4": "User checks give-up",
    "U5": "User gets in",
    "U6": "User gets out",
}


def instrument(shared_state, elevator, obj, names, delayed_names=()):
    for name in names:
        original = getattr(obj, name)
        has_delay = name in delayed_names

        def make_wrapper(original=original, name=name, has_delay=has_delay):
            def log_row(user=None):
                if name == "U2" and user is not None:
                    giveup_at = shared_state.TIME + user.GIVEUPTIME / UNIT
                    action = (f"{user.NAME} arrives at floor {user.IN}, destination is {user.OUT}, "
                              f"GIVEUP at {giveup_at:.0f}.")
                else:
                    action = ACTIONS.get(name, "")
                    if user is not None:
                        action = f"{user.NAME}: {action}"
                print(f"{shared_state.TIME:7.0f} {state_letter(elevator.STATE):<2} "
                      f"{elevator.FLOOR:<3} {flag(elevator.D1)} {flag(elevator.D2)} "
                      f"{flag(elevator.D3)}  {name:<3} {action}")

            async def wrapper(*args, **kwargs):
                # U2-U6 are called with the user as the first argument; E-steps
                # aren't, so this stays None for them and log_row falls back
                # to the generic action text.
                user = args[0] if args and hasattr(args[0], "NAME") else kwargs.get("user")

                # E5/E9 take an optional leading delay and sleep before doing
                # real work -- log after that sleep, not at creation time,
                # or the row would show the wrong moment (and wrong state).
                if has_delay:
                    delay = args[0] if args else kwargs.get("delay", 0)
                    if delay:
                        await asyncio.sleep(delay)
                    log_row(user)
                    return await original(0)

                if name == "U4":
                    # U4 either gives up (position_node -> None) or stays and
                    # reschedules itself -- only the give-up outcome is worth
                    # its own row, and only after the GIVEUPTIME wait inside
                    # U4 has actually resolved, not at the call that starts it.
                    result = await original(*args, **kwargs)
                    if user is not None and user.position_node is None:
                        print(f"{shared_state.TIME:7.0f} {state_letter(elevator.STATE):<2} "
                              f"{elevator.FLOOR:<3} {flag(elevator.D1)} {flag(elevator.D2)} "
                              f"{flag(elevator.D3)}  U4  {user.NAME} gives up and leaves the system.")
                    return result

                log_row(user)
                return await original(*args, **kwargs)

            return wrapper

        setattr(obj, name, make_wrapper())


async def watch_floor(shared_state, elevator):
    """E7/E8 are single calls with an internal per-floor loop, so wrapping
    the call itself only logs once, at the start of the whole multi-floor
    trip. Table 1 instead shows a fresh row each time the elevator reaches
    a new floor while still moving -- watching FLOOR for changes gets that
    without needing to touch elevator.py's own loop."""
    last_floor = elevator.FLOOR
    while True:
        await asyncio.sleep(UNIT * 5)
        if elevator.FLOOR != last_floor:
            step = "E7" if elevator.FLOOR > last_floor else "E8"
            print(f"{shared_state.TIME:7.0f} {state_letter(elevator.STATE):<2} "
                  f"{elevator.FLOOR:<3} {flag(elevator.D1)} {flag(elevator.D2)} "
                  f"{flag(elevator.D3)}  {step:<3} {ACTIONS.get(step, '')}")
            last_floor = elevator.FLOOR


if __name__ == "__main__":
    from users import Users
    from elevator import Elevator

    MAX_USERS = 2
    shared_state = SharedState(MAX_USERS)
    users_list = generate_users(shared_state, MAX_USERS)

    users = Users(users=users_list)
    elevator = Elevator(shared_state, users)

    print(f"{'TIME':>7} {'ST':<2} {'FLR':<3} D1 D2 D3  STEP ACTION")
    instrument(shared_state, elevator, elevator, ["E1", "E2", "E3", "E4", "E5", "E6"],
               delayed_names=["E5"])
    instrument(shared_state, elevator, users, ["U2", "U3", "U4", "U5", "U6"])

    async def run_simulation(elevator, users):
        asyncio.create_task(watch_floor(shared_state, elevator))
        elevator.tasks[elevator.E1] = asyncio.create_task(elevator.E1())
        await users.U1()

        while not (elevator.is_running(elevator.E1)
                and elevator.SHARED_STATE.ELEVATOR.size == 0
                and all(q.size == 0 for q in elevator.SHARED_STATE.QUEUE)):
            await asyncio.sleep(UNIT)

    asyncio.run(run_simulation(elevator, users))
