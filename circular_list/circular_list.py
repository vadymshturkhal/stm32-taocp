class Node:
        def __init__(self, info=None, link=None):
            self.info = info
            self.link = link

class CircularLinkedList:
    def __init__(self, size):
        self.size = size
        self.avail = None
        self.ptr = None
        self._init_storage_pool()

    def _init_storage_pool(self):
        """Create list of available nodes from end to start"""
        if self.size <= 0:
            return
                
        size = self.size
        avail = Node(info=size)

        size -= 1
        while size > 0:
            avail = Node(size, avail)
            size -= 1

        self.avail = avail
        
    def insert_at_left(self, Y):
        if self.avail is None:
            raise Exception("Overflow")

        P = self.avail
        self.avail = self.avail.link
        
        P.info = Y
        
        if self.ptr is None:
            P.link = P
            self.ptr = P
        else:
            P.link = self.ptr.link
            self.ptr.link = P
        
    def insert_at_right(self, Y):
        self.insert_at_left(Y)
        self.ptr = self.ptr.link
        
    def pop_left(self):
        if self.ptr is None:
            raise Exception("Underflow")

        P = self.ptr.link
        Y = P.info

        if self.ptr == P:
            self.ptr = None
        else:
            self.ptr.link = P.link
        
        P.link = self.avail
        self.avail = P
        
        return Y
        
    def clear(self):
        if self.ptr is not None:
            P = self.avail
            self.avail = self.ptr.link
            self.ptr.link = P
            self.ptr = None
    
    def union(self, circular_linked_list):
        """Insert the entire circular_linked_list at the right of current circular linked list"""
        if circular_linked_list.ptr is None:
            return
        
        if self.ptr is not None:
            P = self.ptr.link
            self.ptr.link = circular_linked_list.ptr.link
            circular_linked_list.ptr.link = P
        
        self.ptr = circular_linked_list.ptr
        circular_linked_list.ptr = None
        
        
            
        