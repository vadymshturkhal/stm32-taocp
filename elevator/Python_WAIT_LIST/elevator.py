from settings import ElevatorNode, WaitInfo
from settings import holdc, hold, cycle1, deletew
from settings import INITIAL_FLOOR, FLOORS, CALLUP, CALLDOWN, CALLCAR

class Elevator:
    def __init__(self, shared_state):
        self.shared_state = shared_state
        shared_state.elevator = self  # add Elevator to shared state
        self.FLOOR = INITIAL_FLOOR
        self.STATE = 0  # Neutral
        self.D1 = 0  # a variable that is zero except during the time people are getting in or out of the elevator
        self.D2 = 0  # a variable that becomes zero if the elevator has sat on one floor without moving for 30 sec or more
        self.D3 = 0  # a variable that is zero except when the doors are open but nobody is getting in or out of the elevator
        self.home_floor = INITIAL_FLOOR

        self.ELEV1 = ElevatorNode(info=WaitInfo(NEXTINST=self.E1))  # This node represents the elevator actions, except for E5 and E9
        self.ELEV2 = ElevatorNode(info=WaitInfo(NEXTINST=self.E5))  # Represents the independent elevator action at E5
        self.ELEV3 = ElevatorNode(info=WaitInfo(NEXTINST=self.E9))  # Represents the independent elevator action at E9

    def E1A(self):
        """
        Set NEXTINST = E1 and go to CYCLE
        """
        # JMP CYCLE1
        cycle1(self.ELEV1, self.E1)

    # [Wait for call]
    def E1(self, node):
        pass

    def E2A(self, node, delay):
        # JMP HOLDC
        holdc(self.shared_state, node, delay, self.E2)

    # [Change of state?]
    def E2(self):
        # 1 STATE is GOINGUP
        if self.STATE > 0:
            # Are there calls for higher floors?
            if any(self.shared_state.CALLS[j] != 0 for j in range(self.FLOOR + 1, FLOORS)):
                    # if yes, go to E3
                    self.E3()
                    return

            # Have passengers in the elevator called for lower floors?
            if any(self.shared_state.CALLS[j] & CALLCAR != 0 for j in range(self.FLOOR)):
                # reverse direction of STATE
                self.STATE = -self.STATE
            else:
                self.STATE = 0

        # 2 STATE is GOINGDOWN
        elif self.STATE < 0:
            # Are there calls for lower floors?
            if any(self.shared_state.CALLS[j] != 0 for j in range(self.FLOOR)):
                   # if yes, go to E3
                    self.E3()
                    return

            # Have passengers in the elevator called for higher floors?
            if any(self.shared_state.CALLS[j] & CALLCAR != 0 for j in range(self.FLOOR + 1, FLOORS)):
                # reverse direction of STATE
                self.STATE = -self.STATE
            else:
                self.STATE = 0

        else:  # STATE == NEUTRAL
            raise Exception("Elevator.E2: Trying to change NEUTRAL state")
    
        # Set all CALL variables for the current FLOOR to zero
        self.shared_state.CALLS[self.FLOOR] = 0b000

        # jump to E3
        self.E3()

    # [Open door]
    def E3(self):
        # if activity already scheduled: remove it from WAIT list
        # NOTE: always True
        if self.ELEV3.info.NEXTINST is not None:
            deletew(self.shared_state, self.ELEV3)

        # schedule activity E9 after 300 units
        delay = 300
        hold(self.shared_state, self.ELEV3, delay)

        # schedule activity E5 after 76 units
        delay = 76
        hold(self.shared_state, self.ELEV2, delay)

        # set D2 to nonzero
        self.D2 = 1

        # set D1 to nonzero
        self.D1 = 1

        delay = 20
        self.E4A(self.ELEV1, delay)

    def E4A(self, node, delay):
        # JMP HOLDC
        holdc(self.shared_state, node, delay, self.E4)

    # [Let people out, in]
    def E4(self):
        pass

    def E5A(self, delay):
        # JMP HOLDC
        holdc(self.shared_state, node=self.ELEV2, delay=delay, next_inst=self.E5)

    # [Close door]
    def E5(self):
        pass

    # [Prepare to move]
    def E6(self):
        pass

    def E6B(self):
        if self.STATE == 0:
            # go to E1 and wait
            # NOTE: can skip elf.E1A()
            self.E1A()
            return

    # [Go up a floor]
    def E7(self):
        pass

    # [Go down a floor]
    def E8(self):
        pass

    # [Set inaction indicator]
    def E9(self):
        pass
