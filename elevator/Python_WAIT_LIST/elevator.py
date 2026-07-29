from settings import INITIAL_FLOOR, ElevatorNode, WaitInfo
from settings import holdc

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

        # The elevator needs three WAIT list nodes because it can have three
        # activities pending at once: the main step sequence plus the E5 and E9
        # timers that E3 starts up independently. One node carries a single
        # NEXTTIME/NEXTINST pair, so it could never represent all three.
        # A node sitting outside the WAIT list means that activity isn't
        # scheduled -- which is how "cancel activity E9" is expressed.
        self.ELEV1 = ElevatorNode(info=WaitInfo(NEXTINST=self.E1))  # main sequence, starts dormant
        self.ELEV2 = ElevatorNode(info=WaitInfo(NEXTINST=self.E5))  # close-door timer
        self.ELEV3 = ElevatorNode(info=WaitInfo(NEXTINST=self.E9))  # inaction indicator timer

    # [Wait for call]
    def E1(self):
        pass

    # [Change of state?]
    def E2(self):
        pass

    # [Open door]
    def E3(self):
        pass

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

    # [Go up a floor]
    def E7(self):
        pass

    # [Go down a floor]
    def E8(self):
        pass

    # [Set inaction indicator]
    def E9(self):
        pass
