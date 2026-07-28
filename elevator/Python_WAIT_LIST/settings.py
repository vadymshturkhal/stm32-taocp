import random
import sys
import os

# import DoublyLinkedList and STORAGE_POOL
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "data_structures"))
from storage_pool import STORAGE_POOL


class ElevatorNode:
        def __init__(self, info=None, left=None, right=None, link=None):
            self.info = info
            self.left = left
            self.right = right
            self.link = link

class ElevatorDoublyLinkedList:
    def __init__(self, storage_pool: STORAGE_POOL):
        self.head = storage_pool.pop()
        self.head.left = self.head
        self.head.right = self.head
        self.size = 0
        self.storage_pool = storage_pool

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
    def __init__(self, MAX_USERS: int = 128):
        self.TIME = 0
        self.CALLS = [0b000] * FLOORS
        LINKED_LISTS = 3  # heads
        WAIT_LIST_LEN = 32
        self.storage_pool = STORAGE_POOL(node_class=ElevatorNode, size=MAX_USERS + LINKED_LISTS + FLOORS + WAIT_LIST_LEN)
        self.ELEVATOR = ElevatorDoublyLinkedList(self.storage_pool)                      # Users currently riding the car
        self.QUEUE = [ElevatorDoublyLinkedList(self.storage_pool) for _ in range(FLOORS)]  # Users waiting at each floor
        self.WAIT_LIST = ElevatorDoublyLinkedList(self.storage_pool)

def generate_users(shared_state: SharedState, MAX_USERS: int) -> list:
    users = []
    for i in range(MAX_USERS):
        IN = random.randint(0, FLOORS - 1)
        OUT = random.choice([floor for floor in range(FLOORS) if floor != IN])
        GIVEUPTIME = random.randint(200, 800) * UNIT
        NAME = f"User {i}"
        user = shared_state.storage_pool.pop()
        user.info = UserInfo(shared_state, IN, OUT, GIVEUPTIME, NAME)
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
    MAX_USERS = 4
    shared_state = SharedState(MAX_USERS)
    users_list = generate_users(shared_state, MAX_USERS)
    print(type(users_list[0]), users_list[0].info, users_list[1].info)
