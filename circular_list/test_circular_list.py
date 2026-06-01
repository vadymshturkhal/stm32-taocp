from storage_pool import STORAGE_POOL, Node
from circular_list import CircularLinkedList
import unittest

class TestDecoupledCircularLinkedList(unittest.TestCase):

    def setUp(self):
        """Set up a single storage pool and a primary list instance."""
        self.capacity = 5
        self.pool = STORAGE_POOL(node_class=Node, size=self.capacity)
        self.cll = CircularLinkedList(storage_pool=self.pool)

    def test_initialization(self):
        """Test if the list initializes correctly and handles empty pops."""
        with self.assertRaises(Exception):
            self.cll.pop_left()

    def test_insert_at_left(self):
        """Test inserting at the left (head) and popping left (LIFO behavior)."""
        self.cll.insert_at_left(10)
        self.cll.insert_at_left(20)
        self.cll.insert_at_left(30)
        
        # Should pop in reverse order of insertion (30 -> 20 -> 10)
        self.assertEqual(self.cll.pop_left(), 30)
        self.assertEqual(self.cll.pop_left(), 20)
        self.assertEqual(self.cll.pop_left(), 10)

    def test_insert_at_right(self):
        """Test inserting at the right (tail) and popping left (FIFO / Queue behavior)."""
        self.cll.insert_at_right(1)
        self.cll.insert_at_right(2)
        self.cll.insert_at_right(3)
        
        # Should pop in the exact order of insertion (1 -> 2 -> 3)
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
        """Test clearing the linked list back to the pool in O(1) time."""
        self.cll.insert_at_right(100)
        self.cll.insert_at_right(200)
        
        self.cll.clear()
        
        # List should be empty now
        with self.assertRaises(Exception):
            self.cll.pop_left()

    def test_union_standard(self):
        """Test unioning two lists that share the exact same storage pool."""
        # Both lists consume memory from the same central pool
        cll2 = CircularLinkedList(storage_pool=self.pool)
        
        self.cll.insert_at_right("A")
        self.cll.insert_at_right("B")
        
        cll2.insert_at_right("C")
        cll2.insert_at_right("D")
        
        # Union cll2 into self.cll (Total 4 nodes used out of 5 capacity)
        self.cll.union(cll2)
        
        # Output sequence: A -> B -> C -> D
        self.assertEqual(self.cll.pop_left(), "A")
        self.assertEqual(self.cll.pop_left(), "B")
        self.assertEqual(self.cll.pop_left(), "C")
        self.assertEqual(self.cll.pop_left(), "D")

    def test_union_with_empty(self):
        """Test unioning an empty list into a populated list."""
        empty_cll = CircularLinkedList(storage_pool=self.pool)
        self.cll.insert_at_right(1)
        
        self.cll.union(empty_cll)
        
        # Original list should remain unaffected
        self.assertEqual(self.cll.pop_left(), 1)
        with self.assertRaises(Exception):
            self.cll.pop_left()

    def test_empty_union_with_populated(self):
        """Test unioning a populated list into an empty list."""
        populated_cll = CircularLinkedList(storage_pool=self.pool)
        populated_cll.insert_at_right(99)
        
        self.cll.union(populated_cll)
        self.assertEqual(self.cll.pop_left(), 99)

    def test_capacity_overflow(self):
        """Test inserting beyond the global storage pool constraint."""
        # Fill the pool to its maximum capacity (5 nodes)
        for i in range(self.capacity):
            self.cll.insert_at_right(i)
            
        # The 6th node request should throw a contextually accurate Memory Overflow exception
        with self.assertRaisesRegex(Exception, "Memory Overflow: STORAGE POOL is empty"):
            self.cll.insert_at_right(99)


if __name__ == '__main__':
    unittest.main()
