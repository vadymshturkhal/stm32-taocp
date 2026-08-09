import random


FLOORS = 5
INITIAL_FLOOR = 2
UNIT = 1  # tenths of seconds

# Masks
CALLUP = 0b100
CALLDOWN = 0b010
CALLCAR = 0b001

class ElevatorNode:
        def __init__(self, info=None, left1=None, right1=None, left2=None, right2=None):
            self.info = info
            self.left1 = left1
            self.right1 = right1
            self.left2 = left2
            self.right2 = right2

class ElevatorDoublyLinkedList:
    def __init__(self):
        self.head = ElevatorNode()
        self.head.left1 = self.head
        self.head.right1 = self.head
        self.head.left2 = self.head
        self.head.right2 = self.head

    def insert_node_at_frontw(self, node: ElevatorNode):
        """
        Insert node into WAIT list using LLINK1 and RLINK1
        """   
        # Insert P at Front
        X = self.head
        node.left1 = X
        node.right1 = X.right1
        X.right1.left1 = node
        X.right1 = node

    def insert_node_at_rear(self, node: ElevatorNode):
        Q = self.head.left2
        node.left2 = Q
        self.head.left2 = node
        Q.right2 = node
        node.right2 = self.head

    def delete_nodew(self, node):
        """
        Delete node from WAIT list
        """
        if node is None:
            raise Exception("ElevatorDoublyLinkedList delete_nodew method: trying to delete None node")
        
        if node == self.head:
            raise Exception("ElevatorDoublyLinkedList delete_nodew method: trying to delete Head node")

        node.left1.right1 = node.right1
        node.right1.left1 = node.left1

        # NOTE: Clear links (not in MIX)
        node.left1 = node.right1 = None

    def delete_node(self, node):
        """
        Delete node from its list
        """
        if node is None:
            raise Exception("ElevatorDoublyLinkedList delete_nodew method: trying to delete None node")
        
        if node == self.head:
            raise Exception("ElevatorDoublyLinkedList delete_nodew method: trying to delete Head node")

        P = node.left2
        Q = node.right2
        Q.left2 = P
        P.right2 = Q

class WaitInfo:
    def __init__(self, NEXTTIME=None, NEXTINST=None, INTERTIME=None):
        self.NEXTTIME = NEXTTIME
        self.NEXTINST = NEXTINST
        self.INTERTIME = INTERTIME

class UserInfo(WaitInfo):
    def __init__(self, SHARED_STATE, IN, OUT, GIVEUPTIME, NAME, **kwargs):
        super().__init__(**kwargs)
        self.SHARED_STATE = SHARED_STATE
        self.IN = IN
        self.OUT = OUT
        self.GIVEUPTIME = GIVEUPTIME
        self.NAME = NAME

    def __str__(self):
        return (f"\nUserInfo: \n NAME={self.NAME} \n IN={self.IN} \n OUT={self.OUT} \n "
                f"GIVEUPTIME={self.GIVEUPTIME} \n ")

class SharedState:
    def __init__(self):
        self.TIME = 0
        self.CALLS = [0b000] * FLOORS
        self.ELEVATOR_LIST = ElevatorDoublyLinkedList()                      # Users currently riding the car
        self.QUEUE = [ElevatorDoublyLinkedList() for _ in range(FLOORS)]  # Users waiting at each floor
        self.WAIT_LIST = ElevatorDoublyLinkedList()
        self.elevator = None
        self.users = None

        # set NEXTTIME of head node to 0
        self.WAIT_LIST.head.info = WaitInfo(NEXTTIME=0)

class Values:
    def __init__(self):
        self.IN = random.randint(0, FLOORS - 1)
        self.OUT = random.choice([floor for floor in range(FLOORS) if floor != self.IN])
        self.GIVEUPTIME = random.randint(100, 400) * UNIT
        self.INTERTIME = random.randint(100, 200) * UNIT

    def __str__(self):
        return (f"\nValues: \n IN={self.IN} \n OUT={self.OUT} \n GIVEUPTIME={self.GIVEUPTIME} \n "
                f"INTERTIME={self.INTERTIME} \n ")

def values() -> Values:
    """
    JMP VALUES -- the subroutine U1 calls to fabricate one arriving user's
    IN, OUT, GIVEUPTIME, INTERTIME.
    """
    return Values()

def create_users(shared_state, quantity: int) -> list:
    users = []
    for id in range(quantity):
        user_values = values()
        user_info = UserInfo(shared_state, user_values.IN, user_values.OUT, user_values.GIVEUPTIME,
                            NAME=f"User {id}")
        user = ElevatorNode(info=user_info)
        users.append(user)

    return users

def immed(shared_state: SharedState, wait_node: ElevatorNode):
    """
    Insert wait_node first in WAIT list
    """

    # 1 Set NEXTTIME(wait_node) = TIME
    wait_node.info.NEXTTIME = shared_state.TIME

    # 2 Insert wait_node
    shared_state.WAIT_LIST.insert_node_at_frontw(wait_node)

def insert(list: ElevatorDoublyLinkedList, node: ElevatorNode):
    list.insert_node_at_rear(node)

def deletew(shared_state: SharedState, wait_node: ElevatorNode):
    """
    Delete wait_node from the WAIT list
    """

    shared_state.WAIT_LIST.delete_nodew(wait_node)

def delete(C: ElevatorNode):
    """
    Delete node C from its list
    """

    P = C.left2
    Q = C.right2
    Q.left2 = P
    P.right2 = Q

def sortin(shared_state: SharedState, C: ElevatorNode):
    """
    Sort node C into WAIT list using LLINK1 and RLINK1
    """

    wait_list = shared_state.WAIT_LIST
    P = wait_list.head
    P = P.left1  # last node

    # Compare NEXTTIME fields right to left
    while C.info.NEXTTIME < P.info.NEXTTIME:
        P = P.left1

    # insert to WAIT list
    Q = P.right1
    C.right1 = Q
    C.left1 = P
    P.right1 = C
    Q.left1 = C

def hold(shared_state: SharedState, node: ElevatorNode, delay: int):
    """
    Schedule node to run delay units from now, leaving NEXTINST alone

    U1 needs this variant: USER1 must keep resuming at U1 forever, so its
    NEXTINST has to survive being rescheduled.
    """

    node.info.NEXTTIME = shared_state.TIME + delay
    sortin(shared_state, node)

def holdc(shared_state: SharedState, node: ElevatorNode, delay: int, next_inst):
    """
    hold, and set NEXTINST as well

    MIX takes no argument for next_inst: HOLDC stores rJ, the return address,
    so the node resumes at whatever instruction follows the call -- which is
    why U4 sits directly after U4A's "LDA GIVEUPTIME / JMP HOLDC". Python has
    no return address to capture, so the step gets named instead.
    """

    node.info.NEXTINST = next_inst
    hold(shared_state, node, delay)

def cycle(shared_state: SharedState):
    while True:
            C = shared_state.WAIT_LIST.head.right1
            shared_state.TIME = C.info.NEXTTIME
            deletew(shared_state, C)
            C.info.NEXTINST(C)

def cycle1(node: ElevatorNode, nextinst):
    """
    Set NEXTINST of node
    """
    if not callable(nextinst):
        raise Exception("cycle1: nextinst must be a step, not a node")
    
    node.info.NEXTINST = nextinst
    # JMP CYCLE

def decision(shared_state: SharedState, elevator, caller):
    # D1. Decision necessary?
    if elevator.STATE != 0:
        return

    # D2. Should door open?
    if elevator.ELEV1.info.NEXTINST == elevator.E1 and shared_state.CALLS[INITIAL_FLOOR] != 0:
        # Wait 20 units of time
        delay = 20

        # Schedule E3
        elevator.ELEV1.info.NEXTINST = elevator.E3
        hold(shared_state, elevator.ELEV1, delay)
        return

    # D3. Any calls
    # Search for a nonzero call variable
    j = -1
    for i in range(FLOORS):
        if i == elevator.FLOOR:
            continue

        if elevator.shared_state.CALLS[i] == 0:
            continue

        j = i
        break

    # All CALL[j], j != FLOOR, are zero
    if j == -1:
        # Is caller E6B?
        # NOTE: I decided to use E6 as caller is E6, and MIX version uses E6B
        if caller == elevator.E6:
            # If yes: set j = 2
            j = INITIAL_FLOOR
        else:
            return

    # D4. Set STATE
    # STATE = j - FLOOR
    elevator.STATE = j - elevator.FLOOR

    # JANZ 1B (j = FLOOR not allowed ingeneral): continue in the upper range loop

    # D5. Elevator dormant?
    # Jump if not at E1 or j == 2
    if elevator.ELEV1.info.NEXTINST != elevator.E1 or elevator.STATE == 0:
        return

    # Otherwise schedule E6
    # Wait 20 units of time (same as in D2)
    delay = 20
    elevator.ELEV1.info.NEXTINST = elevator.E6
    hold(shared_state, elevator.ELEV1, delay)
