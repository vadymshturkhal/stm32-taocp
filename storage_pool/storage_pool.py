class Node:
        def __init__(self, info=None, link=None):
            self.info = info
            self.link = link

class STORAGE_POOL:
    """
    "node_class" is any Node class factory;
    The Node class constructor must accept a "link" keyword argument;
    """
    def __init__(self, node_class, size):
        self.node_class = node_class
        self.size = max(0, size)
        self._avail = None
        self._init_storage_pool()

    def _init_storage_pool(self):
        for _ in range(self.size):
            self._avail = self.node_class(link=self._avail)

    def pop(self):
        if self._avail is None:
            raise Exception("Memory Overflow: STORAGE POOL is empty")
        
        P = self._avail
        self._avail = self._avail.link

        return P

    def push(self, node):
        if node is None:
            raise Exception("Pushed null to STORAGE POOL")
    
        node.link = self._avail
        self._avail = node

    def union(self, head, tail):
        """Returns an entire chain to the pool in O(1) time."""
        P = self._avail
        self._avail = head
        tail.link = P
