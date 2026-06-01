class CircularLinkedList:
    """
    Node class used by the storage_pool must have 'link' and 'info' attributes.
    """
    def __init__(self, storage_pool):
        self.storage_pool = storage_pool
        self.ptr = None  # Points to the rightmost/rear node

    def insert_at_left(self, Y):
        P = self.storage_pool.pop()
        P.info = Y
        
        if self.ptr is None:
            P.link = P
            self.ptr = P
        else:
            P.link = self.ptr.link
            self.ptr.link = P
        
    def insert_at_right(self, Y):
        self.insert_at_left(Y)
        self.ptr = self.ptr.link  # Shift rear pointer to the newly added node
        
    def pop_left(self):
        if self.ptr is None:
            raise Exception("List Underflow: CircularLinkedList is empty")

        P = self.ptr.link
        Y = P.info

        if self.ptr == P:
            self.ptr = None
        else:
            self.ptr.link = P.link
        
        self.storage_pool.push(P)        
        return Y
        
    def clear(self):
        if self.ptr is not None:
            self.storage_pool.union(head=self.ptr.link, tail=self.ptr)
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
