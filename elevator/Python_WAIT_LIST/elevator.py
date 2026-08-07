from settings import ElevatorNode, WaitInfo
from settings import holdc, hold, cycle1, deletew, immed, decision
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
    def E1(self, contract_node=None):
        pass

    def E2A(self, node, delay):
        # FIXME: noode parameter is redundant as all methods are using ELEV1 node
        # JMP HOLDC
        holdc(self.shared_state, node, delay, self.E2)

    # [Change of state?]
    def E2(self, contract_node=None):
        elevator = self
        state = "U" if elevator.STATE > 0 else "D" if elevator.STATE < 0 else "N"
        row = (f"{self.shared_state.TIME:04}   {state:<5} {elevator.FLOOR:<4} "
               f"{elevator.D1:<2} {elevator.D2:<2} {elevator.D3:<2}  E2 ")
        action = "Elevator stops"
        print(row + action)
    
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
    def E3(self, contract_node=None):
        elevator = self
        state = "U" if elevator.STATE > 0 else "D" if elevator.STATE < 0 else "N"
        row = (f"{self.shared_state.TIME:04}   {state:<5} {elevator.FLOOR:<4} "
               f"{elevator.D1:<2} {elevator.D2:<2} {elevator.D3:<2}  E3 ")
        action = "Elevator doors start to open"
        print(row + action)
    
        # if activity already scheduled: remove it from WAIT list
        if self.ELEV3.left1 is not None:
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
        self.E4A(delay)

    def E4A(self, delay):
        # JMP HOLDC
        holdc(self.shared_state, self.ELEV1, delay, self.E4)

    # [Let people out, in]
    def E4(self, contract_node=None):
        # C = LOC(ELEVATOR)
        C = self.shared_state.ELEVATOR_LIST.head
        C = C.left2

        # Search ELEVATOR list from left to right
        while C != self.shared_state.ELEVATOR_LIST.head:
            # Compare OUT(C) with FLOOR

            # If not equal: continue
            if C.info.OUT != self.FLOOR:
                C = C.left2
                continue

            # Otherwise prepare to send User to U6:
            # Set NEXTINST(C)
            C.info.NEXTINST = self.shared_state.users.U6

            # Put user at front of the WAIT list
            immed(self.shared_state, C)

            # Wait 25 units and repeat E4A
            delay = 25
            self.E4A(delay)
    
            # Return to simulate other events
            return

        # If C == ELEVATOR_LIST.head: search is complete
        C = self.shared_state.QUEUE[self.FLOOR].head
        C = C.right2

        if C != self.shared_state.QUEUE[self.FLOOR].head:
            # Cancel action U4 for this User
            deletew(self.shared_state, C)

            # Prepare to replace U4 by U5
            nextinst = self.shared_state.users.U5

            # Set NEXTINST(C)
            C.info.NEXTINST = nextinst

            # Put User at front of the WAIT list
            immed(self.shared_state, C)

            # Wait 25 units and repeat E4
            delay = 25
            self.E4A(delay)

            # Return to simulate other events
            return

        # if C == self.shared_state.QUEUE[self.FLOOR].head: QUEUE is empty

        # Set D1 = 0
        self.D1 = 0

        # Set D3 nonzero
        self.D3 = 1
            
        # Search is complete
        # FIXME: try all get out and nobody at the floor case
        floor_list = self.shared_state.QUEUE[self.FLOOR]
        elevator_list = self.shared_state.ELEVATOR_LIST
        if floor_list.head == floor_list.head.left2 and elevator_list.head == elevator_list.head.left2:
            elevator = self
            state = "U" if elevator.STATE > 0 else "D" if elevator.STATE < 0 else "N"
            row = (f"{self.shared_state.TIME:04}   {state:<5} {elevator.FLOOR:<4} "
                f"{elevator.D1:<2} {elevator.D2:<2} {elevator.D3:<2}  E4 ")
            action = "Doors open, nobody is there"
            print(row + action)

    def E5A(self, delay):
        # JMP HOLDC
        holdc(self.shared_state, node=self.ELEV2, delay=delay, next_inst=self.E5)

    # [Close door]
    def E5(self, contract_node=None):

    
        # Is D1 == 0?

        # If not, people are getting in or out
        if self.D1 != 0:
            elevator = self
            state = "U" if elevator.STATE > 0 else "D" if elevator.STATE < 0 else "N"
            row = (f"{self.shared_state.TIME:04}   {state:<5} {elevator.FLOOR:<4} "
                f"{elevator.D1:<2} {elevator.D2:<2} {elevator.D3:<2}  E5 ")
            action = "Elevator doors flutter"
            print(row + action)
            # Wait 40 units, repeat E5
            delay = 40
            self.E5A(delay)
            return

        elevator = self
        state = "U" if elevator.STATE > 0 else "D" if elevator.STATE < 0 else "N"
        row = (f"{self.shared_state.TIME:04}   {state:<5} {elevator.FLOOR:<4} "
               f"{elevator.D1:<2} {elevator.D2:<2} {elevator.D3:<2}  E5 ")
        action = "Elevator doors start to close"
        print(row + action)

        # If D1 == 0: set D3 = 0
        self.D3 = 0

        # Wait 20 units, then go to E6
        delay = 20
        holdc(self.shared_state, self.ELEV1, delay, self.E6)

    # [Prepare to move]
    def E6(self, contract_node=None):
        # If STATE != GOINGDOWN: CALLUP and CALLCAR on this floor are reset
        # J5N *+2 / STZ CALL,4(1:3)
        if self.STATE >= 0:
            self.shared_state.CALLS[self.FLOOR] &= ~(CALLUP | CALLCAR)

        # If STATE != GOINGUP: reset CALLCAR and CALLDOWN
        # J5P *+2 / STZ CALL,4(3:5)
        if self.STATE <= 0:
            self.shared_state.CALLS[self.FLOOR] &= ~(CALLCAR | CALLDOWN)
            
        # Perform DECISIONS subroutine
        # J5Z DECISION
        if self.STATE == 0:
            decision(self.shared_state, self, self.E6)

        self.E6B()

    def E6B(self):
        # If STATE == NEUTRAL: go to E1 and wait
        if self.STATE == 0:
            self.E1A()  # NOTE: can skip this line
            return

        if self.D2 != 0:
            # Cancel activity E9
            deletew(self.shared_state, self.ELEV3)

        # Wait 15 units of time
        delay = 15

        # If STATE == GOINGDOWN, go to E8
        if self.STATE < 0:
            holdc(self.shared_state, self.ELEV1, delay, self.E8)
            return

        # Else go to E7
        holdc(self.shared_state, self.ELEV1, delay, self.E7)

    # [Go up a floor]
    def E7(self, contract_node=None):
        elevator = self
        state = "U" if elevator.STATE > 0 else "D" if elevator.STATE < 0 else "N"
        row = (f"{self.shared_state.TIME:04}   {state:<5} {elevator.FLOOR:<4} "
               f"{elevator.D1:<2} {elevator.D2:<2} {elevator.D3:<2}  E7 ")
        action = "Elevator moving up"
        print(row + action)
    
        # INC4 1
        self.FLOOR += 1

        # Wait 51 units
        delay = 51
        holdc(self.shared_state, self.ELEV1, delay, next_inst=self.E7_continue)

    # Not in MIX
    def E7_continue(self, contract_node=None):
        # Is CALLCAR[FLOOR] or CALLUP[FLOOR] != 0
        is_callcar_or_callup = self.shared_state.CALLS[self.FLOOR] & (CALLCAR | CALLUP)

        # If yes: it is time to stop elevator
        if is_callcar_or_callup:
            # Wait 14 units and go to E2
            delay = 14
            self.E2A(self.ELEV1, delay)
            return

        # If not
        # Is FLOOR == 2 or CALLDOWN[FLOOR] != 0?
        if self.FLOOR == self.home_floor or self.shared_state.CALLS[self.FLOOR] & CALLDOWN:
            # Are there calls for higher floors?
            if any(self.shared_state.CALLS[j] != 0 for j in range(self.FLOOR + 1, FLOORS)):
                # If yes: repeat E7
                self.E7()
                return
        else:
            # repeat E7
            self.E7()
            return

        # It's time to stop elevator
        delay = 14
        self.E2A(self.ELEV1, delay)        

    # [Go down a floor]
    def E8(self, contract_node=None):
        elevator = self
        state = "U" if elevator.STATE > 0 else "D" if elevator.STATE < 0 else "N"
        row = (f"{self.shared_state.TIME:04}   {state:<5} {elevator.FLOOR:<4} "
               f"{elevator.D1:<2} {elevator.D2:<2} {elevator.D3:<2}  E8 ")
        action = "Elevator moving down"
        print(row + action)

        self.FLOOR -= 1

        # Wait 61 units
        delay = 61
        holdc(self.shared_state, self.ELEV1, delay, next_inst=self.E8_continue)

    # Not in MIX
    def E8_continue(self, contract_node=None):
        # Is CALLCAR[FLOOR] or CALLDOWN[FLOOR] != 0
        is_callcar_or_calldown = self.shared_state.CALLS[self.FLOOR] & (CALLCAR | CALLDOWN)

        # If yes: it is time to stop elevator
        if is_callcar_or_calldown:
            # Wait 23 units and go to E2
            delay = 23
            self.E2A(self.ELEV1, delay)
            return

        # If not
        # Is FLOOR == 2 or CALLUP[FLOOR] != 0?
        if self.FLOOR == self.home_floor or self.shared_state.CALLS[self.FLOOR] & CALLUP:
            # Are there calls for lower floors?
            if any(self.shared_state.CALLS[j] != 0 for j in range(self.FLOOR)):
                # If yes: repeat E8
                self.E8()
                return
        else:
            # repeat E8
            self.E8()
            return

        # It's time to stop elevator
        delay = 23
        self.E2A(self.ELEV1, delay)      

    # [Set inaction indicator]
    def E9(self, contract_node=None):
        # Set D2 = 0
        self.D2 = 0
        decision(self.shared_state, self, self.E9)
