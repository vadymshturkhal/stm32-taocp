"""
Elevator WAIT list discrete simulation program.
This version based on subroutines instead of coroutines and works without any 'yield' or 'async'.
"""

from settings import SharedState, deletew, create_users
from elevator import Elevator
from users import Users


if __name__ == "__main__":
    shared_state = SharedState()
    users_quantity = 4
    users_list = create_users(shared_state, users_quantity)
    users = Users(shared_state, users_list)
    elevator = Elevator(shared_state)
    users.start()

    print("TIME STATE FLOOR D1 D2 D3 step action")

    while shared_state.WAIT_LIST.head.right1 is not shared_state.WAIT_LIST.head:
        C = shared_state.WAIT_LIST.head.right1
        shared_state.TIME = C.info.NEXTTIME
        deletew(shared_state, C)
        C.info.NEXTINST(C)
