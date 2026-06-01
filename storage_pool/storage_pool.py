class STORAGE_POOL:
    """
    "node_class" is any Node class factory;
    The Node class constructor must accept a "link" keyword argument;
    """
    def __init__(self, node_class, size):
        self.node_class = node_class
        self.size = max(0, size)
        self.avail = None
        self._init_storage_pool()

    def _init_storage_pool(self):
        for _ in range(self.size):
            self.avail = self.node_class(link=self.avail)

    def pop(self):
        if self.avail is None:
            raise Exception("Memory Overflow: STORAGE POOL is empty")
        
        P = self.avail
        self.avail = self.avail.link

        return P

    def push(self, node):
        if node is None:
            raise Exception("Pushed null to STORAGE POOL")
    
        node.link = self.avail
        self.avail = node
