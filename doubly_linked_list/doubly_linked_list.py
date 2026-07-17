from storage_pool import STORAGE_POOL


class DoublyNode:
        def __init__(self, info=None, left=None, right=None, link=None):
            self.info = info
            self.left = left
            self.right = right
            self.link = link

        # @property
        # def link(self):
        #     return self.left
        
        # @link.setter
        # def link(self, node):
        #     self.left = node

class DoublyLinkedList:
    """
    Node class used by the storage_pool must have 'link' and 'info' attributes.
    """
    def __init__(self, storage_pool: STORAGE_POOL):
        self.head = storage_pool.pop()
        self.head.left = self.head
        self.head.right = self.head
        self.size = 0
        self.storage_pool = storage_pool

    def insert_left(self, Y):
        # P <= Avail
        P = self.storage_pool.pop()
        P.info = Y

        # Increment size
        self.size += 1
        
        # Insert P at Front
        X = self.head
        P.left = X
        P.right = X.right
        X.right.left = P
        X.right = P
        
    def insert_right(self, Y):
        # P <= Avail
        P = self.storage_pool.pop()
        P.info = Y

        # Increment size
        self.size += 1
        
        # Insert P at Front
        X = self.head.left
        P.left = X
        P.right = X.right
        X.right.left = P
        X.right = P
        
    def pop_left(self):
        if self.head == self.head.right:
            raise Exception("List Underflow: Doubly Linked List is empty")

        # Decrement size
        self.size -= 1

        X = self.head.right
        X.left.right = X.right
        X.right.left = X.left
        
        info = X.info
        self.storage_pool.push(X)
        return info
    
    def pop_right(self):
        if self.head == self.head.right:
            raise Exception("List Underflow: Doubly Linked List is empty")

        # Decrement size
        self.size -= 1

        X = self.head.left
        X.left.right = X.right
        X.right.left = X.left
        
        info = X.info
        self.storage_pool.push(X)
        return info
    
    def insert_node(self, X):
        if X is None:
            raise Exception("Doubly Linked List insert_node method: trying to insert node next to None")

        # P <= Avail
        P = self.storage_pool.pop()

        # Increment size
        self.size += 1
        
        # Insert node
        P.left = X
        P.right = X.right
        X.right.left = P
        X.right = P

    def insert_node(self, X):
        if X is None:
            raise Exception("Doubly Linked List insert_node method: trying to insert node next to None")

        # P <= Avail
        P = self.storage_pool.pop()

        # Increment size
        self.size += 1
        
        # Insert node
        P.left = X
        P.right = X.right
        X.right.left = P
        X.right = P

    def delete_node(self, X):
        if X is None:
            raise Exception("Doubly Linked List delete_node method: trying to delete None node")
        
        if X == self.head:
            raise Exception("Doubly Linked List delete_node method: trying to delete Head node")

        # Decrement size
        self.size -= 1

        X.left.right = X.right
        X.right.left = X.left
        
        # Avail <= X
        self.storage_pool.push(X)
        
    def clear(self):
        if self.head.left == self.head:
            return

        head = self.head.right
        tail = self.head.left

        self.storage_pool.add_slice(head, tail, self.size)
        self.head.left = self.head
        self.head.right = self.head
        self.size = 0
    
    def union(self, list_b: "DoublyLinkedList"):
        if list_b is None:
            return
        
        # list_b is empty
        if list_b.head == list_b.head.left:
            return
        
        # union same list
        if self == list_b:
            return
        
        list_b_start = list_b.head.right
        list_b_end = list_b.head.left
        list_a_end = self.head.left

        # link end with start
        list_a_end.right = list_b_start
        list_b_start.left = list_a_end

        # link all with head
        list_b_end.right = self.head
        self.head.left = list_b_end

        # advance size
        self.size += list_b.size

        # empty list_b
        list_b.head.left = list_b.head
        list_b.head.right = list_b.head
        list_b.size = 0


if __name__ == "__main__":
    nodes = 128 + 1
    storage_pool = STORAGE_POOL(node_class=DoublyNode, size=nodes)
    doubly_linked_list = DoublyLinkedList(storage_pool)
    n = DoublyNode(info=1)
    n1 = DoublyNode(info=2)
    n2 = DoublyNode(info=3)

    doubly_linked_list.insert_left(n)
    doubly_linked_list.insert_left(n1)
    doubly_linked_list.insert_left(n2)
    print(doubly_linked_list.size)
    print(storage_pool.size)

    doubly_linked_list.clear()
    print(doubly_linked_list.size)
    print(storage_pool.size)
