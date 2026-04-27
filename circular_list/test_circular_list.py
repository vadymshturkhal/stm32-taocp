from circular_list import CircularLinkedList
import unittest

# Assuming your class is imported here
# from your_module import CircularLinkedList

class TestCircularLinkedList(unittest.TestCase):

    def setUp(self):
        """Set up fresh instances before each test."""
        self.capacity = 5
        self.cll = CircularLinkedList(size=self.capacity)

    def test_initialization(self):
        """Test if the list initializes correctly."""
        # Assuming your class exposes a way to check current size/length
        # self.assertEqual(len(self.cll), 0)
        
        # Popping from a brand new list should raise an error or return None
        with self.assertRaises(Exception):
            self.cll.pop_left()

    def test_insert_at_left(self):
        """Test inserting at the left (head) and popping left (LIFO behavior)."""
        self.cll.insert_at_left(10)
        self.cll.insert_at_left(20)
        self.cll.insert_at_left(30)
        
        # Should pop in reverse order of insertion (30, then 20, then 10)
        self.assertEqual(self.cll.pop_left(), 30)
        self.assertEqual(self.cll.pop_left(), 20)
        self.assertEqual(self.cll.pop_left(), 10)

    def test_insert_at_right(self):
        """Test inserting at the right (tail) and popping left (FIFO / Queue behavior)."""
        self.cll.insert_at_right(1)
        self.cll.insert_at_right(2)
        self.cll.insert_at_right(3)
        
        # Should pop in the exact order of insertion (1, then 2, then 3)
        self.assertEqual(self.cll.pop_left(), 1)
        self.assertEqual(self.cll.pop_left(), 2)
        self.assertEqual(self.cll.pop_left(), 3)

    def test_mixed_insertions(self):
        """Test a combination of left and right insertions."""
        self.cll.insert_at_right(2)  # List: [2]
        self.cll.insert_at_left(1)   # List: [1, 2]
        self.cll.insert_at_right(3)  # List: [1, 2, 3]
        
        self.assertEqual(self.cll.pop_left(), 1)
        self.assertEqual(self.cll.pop_left(), 2)
        self.assertEqual(self.cll.pop_left(), 3)

    def test_clear(self):
        """Test clearing the linked list."""
        self.cll.insert_at_right(100)
        self.cll.insert_at_right(200)
        
        self.cll.clear()
        
        # List should be empty now
        with self.assertRaises(Exception):
            self.cll.pop_left()

    def test_union_standard(self):
        """Test unioning two non-empty circular linked lists."""
        cll2 = CircularLinkedList(size=3)
        
        self.cll.insert_at_right("A")
        self.cll.insert_at_right("B")
        
        cll2.insert_at_right("C")
        cll2.insert_at_right("D")
        
        # Union cll2 into self.cll
        self.cll.union(cll2)
        
        # Output should be A -> B -> C -> D
        self.assertEqual(self.cll.pop_left(), "A")
        self.assertEqual(self.cll.pop_left(), "B")
        self.assertEqual(self.cll.pop_left(), "C")
        self.assertEqual(self.cll.pop_left(), "D")

    def test_union_with_empty(self):
        """Test unioning an empty list into a populated list."""
        empty_cll = CircularLinkedList(size=5)
        self.cll.insert_at_right(1)
        
        self.cll.union(empty_cll)
        
        # Original list should remain unaffected
        self.assertEqual(self.cll.pop_left(), 1)
        with self.assertRaises(Exception):
            self.cll.pop_left()

    def test_empty_union_with_populated(self):
        """Test unioning a populated list into an empty list."""
        populated_cll = CircularLinkedList(size=5)
        populated_cll.insert_at_right(99)
        
        self.cll.union(populated_cll)
        self.assertEqual(self.cll.pop_left(), 99)

    def test_capacity_overflow(self):
        """Test inserting beyond the initialized size constraint."""
        # Fill the list to capacity (5)
        for i in range(self.capacity):
            self.cll.insert_at_right(i)
            
        # The 6th insertion should raise an exception
        with self.assertRaises(Exception): # Replace Exception with your specific error
            self.cll.insert_at_right(99)
            

if __name__ == '__main__':
    unittest.main()
