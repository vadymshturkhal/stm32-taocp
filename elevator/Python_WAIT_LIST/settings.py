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
    def __init__(self, SHARED_STATE, IN, OUT, GIVEUPTIME, NAME=None):
        self.SHARED_STATE = SHARED_STATE
        self.IN = IN
        self.OUT = OUT
        self.GIVEUPTIME = GIVEUPTIME
        self.task = None            # this user's single pending asyncio.Task, if any
        self.arrival_time = None    # SHARED_STATE.TIME when this user entered the system
        self.NAME = NAME

    def __str__(self):
        return (f"\nUserInfo: \n NAME={self.NAME} \n IN={self.IN} \n OUT={self.OUT} \n "
                f"GIVEUPTIME={self.GIVEUPTIME} \n task={self.task} \n arrival_time={self.arrival_time} \n ")

class WaitInfo:
    def __init__(self, NEXTTIME=None, NEXTINST=None):
        """
        Here LLINK1 and RLINK1 are the links for the WAIT list
        LLINK2 and RLINK2 are used as links in the QUEUE list or the ELEVATOR
        """
        self.NEXTTIME = NEXTTIME
        self.NEXTINST = NEXTINST

class SharedState:
    def __init__(self):
        self.TIME = 0
        self.CALLS = [0b000] * FLOORS
        self.ELEVATOR = ElevatorDoublyLinkedList()                      # Users currently riding the car
        self.QUEUE = [ElevatorDoublyLinkedList() for _ in range(FLOORS)]  # Users waiting at each floor
        self.WAIT_LIST = ElevatorDoublyLinkedList()

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

def immed(wait_node: ElevatorNode, shared_state: SharedState):
    """
    Insert wait_node first in WAIT list
    """

    # 1 Set NEXTTIME(wait_node) = TIME
    wait_node.info.NEXTTIME = shared_state.TIME

    # 2 Insert wait_node
    shared_state.WAIT_LIST.insert_node_at_front(wait_node)


if __name__ == "__main__":
    max_users = 4
    shared_state = SharedState()
    users_list = generate_users(shared_state, max_users)
    print(type(users_list[0]), users_list[0].info, users_list[1].info)
