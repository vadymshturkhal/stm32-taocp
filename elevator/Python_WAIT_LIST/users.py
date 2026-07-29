from settings import Values, UserInfo, WaitInfo, ElevatorNode
from settings import hold, immed, deletew, decision
from settings import CALLUP, CALLDOWN, CALLCAR


class Users:
    def __init__(self, shared_state):
        self.shared_state = shared_state
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

            # NOTE : Knuth is using Node ELEV1 and replaced only NEXTINST field
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
            decision(elevator)

        self.U3(user)

    # [Enter queue]
    def U3(self, user: ElevatorNode):
        """
        Insert node at right end of QUEUE[IN]
        """

        # Instead of self.shared_state.QUEUE[user.info.IN].insert(user, QUEUE[IN].head)
        self.shared_state.QUEUE[user.info.IN].insert_node_at_rear(user)
        self.U4A(user)

    # [Wait GIVEUPTIME units]
    def U4A(self, user):
        # LDA GIVEUPTIME
        # HOLDC
        user.info.NEXTINST = self.U4
        hold(self.shared_state, node=user, delay=user.info.GIVEUPTIME)

    # [Give up]
    def U4(self):
        pass

    # [Get in]
    def U5(self):
        pass

    # [Get out]
    def U6(self):
        pass
