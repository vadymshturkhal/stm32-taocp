from settings import Values, UserInfo, ElevatorNode, WaitInfo
from settings import hold, immed, deletew, decision
from settings import CALLUP, CALLDOWN, CALLCAR


class Users:
    def __init__(self, shared_state):
        self.shared_state = shared_state
        self.user_id = 0  # not in Coroutine U

    # [Enter, prepare for successor]
    def U1(self):
        # 1 compute VALUES
        values = Values()
        user_info = UserInfo(self.shared_state, values.IN, values.OUT, values.GIVEUPTIME, NAME=f"User {self.user_id}")
        user = ElevatorNode(info=user_info)

        # 2 put node in WAIT list with INTERTIME delay
        wait_info = WaitInfo(NEXTINST=self.U1, INTERTIME=values.INTERTIME)
        hold(self.shared_state, ElevatorNode(info=wait_info))

        # increment user_id 
        self.user_id += 1

        # to U2
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
        pass

    def U4(self):
        pass

    def U5(self):
        pass

    def U6(self):
        pass
