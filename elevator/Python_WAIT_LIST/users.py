from settings import Values, UserInfo, WaitInfo, ElevatorNode
from settings import hold, holdc, immed, insert, deletew, delete, decision
from settings import CALLUP, CALLDOWN, CALLCAR
from settings import UNIT


class Users:
    def __init__(self, shared_state):
        self.shared_state = shared_state
        shared_state.users = self
        self.user_id = 0  # not in Coroutine U

        # USER1 is to the users what ELEV1/ELEV2/ELEV3 are to the elevator: a
        # fixed node standing for one activity, here U1. It re-holds itself on
        # every U1, so once started it keeps the arrivals coming.
        self.USER1 = ElevatorNode(info=WaitInfo(NEXTINST=self.U1))

    def start(self):
        """
        USER1 node represents action U1 and it is initially the sole entry in the WAIT list
        """
        immed(self.shared_state, self.USER1)

    # [Enter, prepare for successor]
    def U1(self, C: ElevatorNode):
        # 1 JMP VALUES
        values = Values()

        # 2 
        # LDA INTERTIME
        # JMP HOLD
        # Put node in WAIT, delay INTERTIME
        hold(self.shared_state, node=C, delay=values.INTERTIME)

        # 3 Create User
        user_info = UserInfo(self.shared_state, values.IN, values.OUT, values.GIVEUPTIME,
                             NAME=f"User {self.user_id}")
        user = ElevatorNode(info=user_info)

        # 4 increment user_id (not in Coroutine U)
        self.user_id += 1

        # 5 to U2, with C now the new node
        self.U2(user)

    # [Signal and wait]
    def U2(self, user: ElevatorNode):
        elevator = self.shared_state.elevator

        # 1
        # if FLOOR == IN
        if elevator.FLOOR == user.info.IN:
            # 2 
            # and if the elevator's next action is step E6 
            # (if the elevator doors are now closing)

            if elevator.ELEV1.info.NEXTINST == elevator.E6:
                # reposition it at E3
                elevator.ELEV1.info.NEXTINST = elevator.E3

                # JMP DELETEW
                deletew(self.shared_state, elevator.ELEV1)

                # JMP IMMED
                immed(self.shared_state, elevator.ELEV1)

                # JMP U3
                self.U3(user)
                return

            # 3 
            # if D3 != 0
            if elevator.D3 != 0:
                # set D3 = 0, D1 to a nonzero value and start up the elevator's activity E4 again
                elevator.D1 = 1
                elevator.D3 = 0

                # NOTE: no NEXTINST store and no DELETEW here -- both rely on
                # the state E4 leaves behind when it finds nobody waiting: it
                # sets D1 <- 0, D3 <- nonzero and returns WITHOUT rescheduling,
                # so ELEV1 is out of the WAIT list with NEXTINST still E4.
                # MIX 127-131 leans on exactly the same thing. Should E4 ever
                # return leaving some other NEXTINST, this silently restarts
                # the wrong step; should it return still scheduled, immed
                # double-inserts ELEV1 and corrupts the list. Neither fails
                # loudly, and the damage shows up here rather than in E4.
                # JMP IMMED
                immed(self.shared_state, elevator.ELEV1)

                # JMP U3
                self.U3(user)
                return

        # 4 
        # in all other cases the user sets CALLUP[IN] = 1 or CALLDOWN[IN] = 1
        # according as OUT > IN or OUT < IN
        if user.info.OUT > user.info.IN:
            self.shared_state.CALLS[user.info.IN] |= CALLUP
        else:
            self.shared_state.CALLS[user.info.IN] |= CALLDOWN

        # 5 
        # and if D2 == 0 or the elevator in its "dormant" position E1, the DESICION performed
        if elevator.D2 == 0 or elevator.ELEV1.info.NEXTINST == elevator.E1:
            decision(self.shared_state, elevator, caller=self.U2)

        self.U3(user)

    # [Enter queue]
    def U3(self, user: ElevatorNode):
        """
        Insert node at right end of QUEUE[IN]
        """

        insert(self.shared_state.QUEUE[user.info.IN], user)
        self.U4A(user)

    # [Wait GIVEUPTIME units]
    def U4A(self, user):
        # LDA GIVEUPTIME
        # JMP HOLDC
        holdc(self.shared_state, node=user, delay=user.info.GIVEUPTIME, next_inst=self.U4)

    # [Give up]
    def U4(self, user):
        elevator = self.shared_state.elevator

        if user.info.IN - elevator.FLOOR != 0:
            self.U6(user)
            return

        if elevator.D1 != 0:
            # JANZ U4A
            self.U4A(user)
            return

        self.U6(user)

    # [Get in]
    def U5(self, user):
        elevator = self.shared_state.elevator

        # 1
        # This user now leaves QUEUE and enters ELEVATOR

        # Delete User from QUEUE[IN]
        delete(user)

        # Insert it at right of ELEVATOR
        insert(self.shared_state.ELEVATOR_LIST, user)

        # 2 Set CALLCAR[OUT] = 1
        self.shared_state.CALLS[user.info.OUT] |= CALLCAR

        # 3
        # if STATE == NEUTRAL, set STATE = GOINGUP or GOINGDOWN as appropriate
        # Knuth's style
        if elevator.STATE == 0:
            elevator.STATE = user.info.OUT - elevator.FLOOR

        # 4
        # Set elevator's activity E5 to be executed after 25 units of time
        # Remove action E5 from WAIT list
        deletew(self.shared_state, elevator.ELEV2)

        # Restart E5 after 25 units
        elevator.E5A(delay=UNIT * 25)

    # [Get out]
    def U6(self, user):
        # JMP DELETE
        delete(user)
