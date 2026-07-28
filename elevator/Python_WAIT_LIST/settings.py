import random


class ElevatorNode:
        def __init__(self, info=None, left=None, right=None, link=None):
            self.info = info
            self.left = left
            self.right = right
            self.link = link

class ElevatorDoublyLinkedList:
    def __init__(self):
        self.head = ElevatorNode()
        self.head.left = self.head
        self.head.right = self.head
        self.size = 0

    def insert_node_at_front(self, node: ElevatorNode):
        # Increment size
        self.size += 1
        
        # Insert P at Front
        X = self.head
        node.left = X
        node.right = X.right
        X.right.left = node
        X.right = node

    def delete_node(self, node):
        if node is None:
            raise Exception("Doubly Linked List delete_node method: trying to delete None node")
        
        if node == self.head:
            raise Exception("Doubly Linked List delete_node method: trying to delete Head node")

        # Decrement size
        self.size -= 1

        node.left.right = node.right
        node.right.left = node.left


FLOORS = 5
INITIAL_FLOOR = 2
UNIT = 1  # tenths of seconds

# Masks
CALLUP = 0b100
CALLDOWN = 0b010
CALLCAR = 0b001

class UserInfo:
    def __init__(self, SHARED_STATE, IN, OUT, GIVEUPTIME, NAME):
        self.SHARED_STATE = SHARED_STATE
        self.IN = IN
        self.OUT = OUT
        self.GIVEUPTIME = GIVEUPTIME
        self.NAME = NAME

    def __str__(self):
        return (f"\nUserInfo: \n NAME={self.NAME} \n IN={self.IN} \n OUT={self.OUT} \n "
                f"GIVEUPTIME={self.GIVEUPTIME} \n ")

class WaitInfo:
    def __init__(self, NEXTTIME=None, NEXTINST=None, INTERTIME=None):
        self.NEXTTIME = NEXTTIME
        self.NEXTINST = NEXTINST
        self.INTERTIME = INTERTIME

class SharedState:
    def __init__(self):
        self.TIME = 0
        self.CALLS = [0b000] * FLOORS
        self.ELEVATOR_LIST = ElevatorDoublyLinkedList()                      # Users currently riding the car
        self.QUEUE = [ElevatorDoublyLinkedList() for _ in range(FLOORS)]  # Users waiting at each floor
        self.WAIT_LIST = ElevatorDoublyLinkedList()
        self.elevator = None

        # set NEXTTIME of head node to 0
        self.WAIT_LIST.head.info = WaitInfo(NEXTTIME=0)

class Values:
    def __init__(self):
        self.IN = random.randint(0, FLOORS - 1)
        self.OUT = random.choice([floor for floor in range(FLOORS) if floor != self.IN])
        self.GIVEUPTIME = random.randint(200, 800) * UNIT
        self.INTERTIME = random.randint(100, 1600) * UNIT

    def __str__(self):
        return (f"\nValues: \n IN={self.IN} \n OUT={self.OUT} \n GIVEUPTIME={self.GIVEUPTIME} \n "
                f"INTERTIME={self.INTERTIME} \n ")

def generate_users(shared_state: SharedState, max_users: int) -> list:
    users = []
    for i in range(max_users):
        IN = random.randint(0, FLOORS - 1)
        OUT = random.choice([floor for floor in range(FLOORS) if floor != IN])
        GIVEUPTIME = random.randint(200, 800) * UNIT
        NAME = f"User {i}"
        user = ElevatorNode(UserInfo(shared_state, IN, OUT, GIVEUPTIME, NAME))
        users.append(user)
    return users

def immed(shared_state: SharedState, wait_node: ElevatorNode):
    """
    Insert wait_node first in WAIT list
    """

    # 1 Set NEXTTIME(wait_node) = TIME
    wait_node.info.NEXTTIME = shared_state.TIME

    # 2 Insert wait_node
    shared_state.WAIT_LIST.insert_node_at_front(wait_node)

def deletew(shared_state: SharedState, wait_node: ElevatorNode):
    """
    Delete wait_node from the WAIT list

    This is how a scheduled activity is cancelled. One node stands for one
    activity, not one entity -- the elevator keeps three (ELEV1, ELEV2, ELEV3)
    because E3 starts the E5 and E9 timers up independently of the main step
    sequence, so all three can be pending at once. Removing a node means that
    one activity is no longer scheduled; the node itself survives with
    NEXTINST intact, ready to be repositioned and reinserted by immed or hold.
    """

    shared_state.WAIT_LIST.delete_node(wait_node)

def sortin(shared_state: SharedState, C: ElevatorNode):
    """
    Sort node C into WAIT list
    Thee cases: insert C at Front, at Middle, at the Rear

    C is a node the caller owns, not one created here: the elevator's ELEV1,
    ELEV2 and ELEV3 are fixed records that get inserted and removed over and
    over, so deletew has to be able to find the same object it scheduled.
    """

    wait_list = shared_state.WAIT_LIST
    P = wait_list.head
    P = P.left  # last node

    # Compare NEXTTIME fields right to left
    while C.info.NEXTTIME < P.info.NEXTTIME:
        P = P.left

    # insert to WAIT list
    Q = P.right
    C.right = Q
    C.left = P
    P.right = C
    Q.left = C

    wait_list.size += 1

def hold(shared_state: SharedState, C: ElevatorNode):
    C.info.NEXTTIME = shared_state.TIME + C.info.INTERTIME
    return sortin(shared_state, C)

def decision(elevator):
    pass
    
if __name__ == "__main__":
    max_users = 4
    shared_state = SharedState()
    users_list = generate_users(shared_state, max_users)
    print(type(users_list[0]), users_list[0].info, users_list[1].info)

    v = Values()
    print(v)