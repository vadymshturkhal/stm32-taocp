"""
Elevator WAIT list discrete simulation program.
This version based on subroutines instead of coroutines and works without any 'yield' or 'async' keywords.

Run: python3 main.py
Edit users_quantity below to change the number of arrivals, 
or comment out random.seed(1) for a different run each time.

The dispatcher is Knuth's CYCLE: take the earliest node off the WAIT list,
advance TIME to its NEXTTIME, unlink it, and call NEXTINST. Everything else
is the steps rescheduling themselves; the loop below just repeats that until
the WAIT list runs dry.

Trace columns:
    TIME    simulated time, in tenths of seconds (UNIT)
    STATE   U/D/N -- elevator going up, down, or neutral
    FLOOR   elevator's current floor
    D1      nonzero while people are getting in or out
    D2      nonzero until 300 units pass with no action (E9's inaction timer)
    D3      nonzero while the doors are open but nobody is getting in or out
    step    the Knuth step (U1-U6, E1-E9) that produced this row
"""
from settings import SharedState, deletew, create_users, values_seed
from elevator import Elevator
from users import Users


if __name__ == "__main__":
    values_seed(1)  # matches the C port's values_seed(1): same xorshift32 sequence

    shared_state = SharedState()
    users_quantity = 4
    # users_list = create_users(shared_state, users_quantity)
    users = Users(shared_state, users=None, users_quantity=users_quantity)
    elevator = Elevator(shared_state)
    users.start()

    print("TIME STATE FLOOR D1 D2 D3 step action")

    # CYCLE control routine
    while shared_state.WAIT_LIST.head.right1 is not shared_state.WAIT_LIST.head:
        C = shared_state.WAIT_LIST.head.right1
        shared_state.TIME = C.info.NEXTTIME
        deletew(shared_state, C)
        C.info.NEXTINST(C)
