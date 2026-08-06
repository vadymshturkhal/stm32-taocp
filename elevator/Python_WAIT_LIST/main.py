"""
Run the elevator simulation.

    python3 main.py                 400 events, traced
    python3 main.py 5000            5000 events, traced
    python3 main.py 40000 quiet     summary only

The dispatcher is Knuth's CYCLE: take the earliest node off the WAIT list,
advance TIME to its NEXTTIME, unlink it, and call NEXTINST. Everything else
is the steps rescheduling themselves.
"""
import random

from settings import ElevatorNode, SharedState, UserInfo, deletew, values
from elevator import Elevator
from users import Users


ACTIONS = {
    "E1": "Elevator dormant",
    "E2": "Elevator stops",
    "E3": "Elevator doors start to open",
    "E4": "Elevator lets people in/out",
    "E5": "Elevator doors start to close",
    "E6": "Elevator prepares to move",
    "E7": "Elevator moving up",
    "E8": "Elevator moving down",
    "E9": "Elevator sets inaction indicator",
    "U2": "User signals and waits",
    "U3": "User enters queue",
    "U4": "User checks give-up",
    "U5": "User gets in",
    "U6": "User gets out",
}

def create_users(shared_state, quantity: int) -> list:
    users = []
    for id in range(quantity):
        user_values = values()
        user_info = UserInfo(shared_state, user_values.IN, user_values.OUT, user_values.GIVEUPTIME,
                            NAME=f"User {id}")
        user = ElevatorNode(info=user_info)
        users.append(user)

    return users


if __name__ == "__main__":
    shared_state = SharedState()
    users_quantity = 4
    users_list = create_users(shared_state, users_quantity)
    # print(users)
    users = Users(shared_state, users_list)
    elevator = Elevator(shared_state)
    users.start()

    print("TIME STATE FLOOR D1 D2 D3 step action")

    while shared_state.WAIT_LIST.head.right1 is not shared_state.WAIT_LIST.head:
        C = shared_state.WAIT_LIST.head.right1
        shared_state.TIME = C.info.NEXTTIME
        deletew(shared_state, C)
        C.info.NEXTINST(C)

