from storage_pool import STORAGE_POOL
from doubly_linked_list import DoublyLinkedList, DoublyNode
import unittest


class TestDoublyLinkedList(unittest.TestCase):

    def setUp(self):
        """Set up a single storage pool and a primary list instance.

        Pool size is capacity + 1 because the list itself permanently
        occupies one node from the pool as its head, leaving `capacity`
        nodes available for actual data.
        """
        self.capacity = 5
        self.pool = STORAGE_POOL(node_class=DoublyNode, size=self.capacity + 1)
        self.dll = DoublyLinkedList(storage_pool=self.pool)

    def test_initialization(self):
        """Test if the list initializes correctly and handles empty pops from both ends."""
        with self.assertRaises(Exception):
            self.dll.pop_left()
        with self.assertRaises(Exception):
            self.dll.pop_right()

    def test_insert_left(self):
        """Test inserting at the left (head) and popping left (LIFO behavior)."""
        self.dll.insert_left(10)
        self.dll.insert_left(20)
        self.dll.insert_left(30)

        # Should pop in reverse order of insertion (30 -> 20 -> 10)
        self.assertEqual(self.dll.pop_left(), 30)
        self.assertEqual(self.dll.pop_left(), 20)
        self.assertEqual(self.dll.pop_left(), 10)

    def test_insert_right(self):
        """Test inserting at the right (tail) and popping left (FIFO / Queue behavior)."""
        self.dll.insert_right(1)
        self.dll.insert_right(2)
        self.dll.insert_right(3)

        # Should pop in the exact order of insertion (1 -> 2 -> 3)
        self.assertEqual(self.dll.pop_left(), 1)
        self.assertEqual(self.dll.pop_left(), 2)
        self.assertEqual(self.dll.pop_left(), 3)

    def test_pop_right_after_insert_right(self):
        """Test that pop_right behaves as a stack (LIFO) on values inserted at the right."""
        self.dll.insert_right(1)
        self.dll.insert_right(2)
        self.dll.insert_right(3)

        # Should pop in reverse order of insertion (3 -> 2 -> 1)
        self.assertEqual(self.dll.pop_right(), 3)
        self.assertEqual(self.dll.pop_right(), 2)
        self.assertEqual(self.dll.pop_right(), 1)

    def test_pop_right_after_insert_left(self):
        """Test that pop_right behaves as a queue (FIFO) on values inserted at the left."""
        self.dll.insert_left(1)
        self.dll.insert_left(2)
        self.dll.insert_left(3)

        # List (left -> right): 3 -> 2 -> 1, so popping right gives 1 -> 2 -> 3
        self.assertEqual(self.dll.pop_right(), 1)
        self.assertEqual(self.dll.pop_right(), 2)
        self.assertEqual(self.dll.pop_right(), 3)

    def test_mixed_insertions(self):
        """Test a combination of left and right insertions."""
        self.dll.insert_right(2)  # List: [2]
        self.dll.insert_left(1)   # List: [1, 2]
        self.dll.insert_right(3)  # List: [1, 2, 3]

        self.assertEqual(self.dll.pop_left(), 1)
        self.assertEqual(self.dll.pop_left(), 2)
        self.assertEqual(self.dll.pop_left(), 3)

    def test_pop_from_both_ends(self):
        """Test alternating pop_left/pop_right on the same list (deque-style behavior)."""
        self.dll.insert_right(1)
        self.dll.insert_right(2)
        self.dll.insert_right(3)
        self.dll.insert_right(4)
        # List: [1, 2, 3, 4]

        self.assertEqual(self.dll.pop_left(), 1)   # [2, 3, 4]
        self.assertEqual(self.dll.pop_right(), 4)  # [2, 3]
        self.assertEqual(self.dll.pop_left(), 2)   # [3]
        self.assertEqual(self.dll.pop_right(), 3)  # []

        with self.assertRaises(Exception):
            self.dll.pop_left()

    def test_insert_node_raises_on_none(self):
        """Test that insert_node refuses to insert relative to a None reference."""
        with self.assertRaisesRegex(
            Exception,
            "Doubly Linked List insert_node method: trying to insert node next to None"
        ):
            self.dll.insert_node(None)

    def test_insert_node(self):
        """Test inserting a new node (from the pool) to the right of a given node."""
        self.dll.insert_left(1)               # List: [1]
        left_node = self.dll.head.right       # node holding 1
        self.dll.insert_node(left_node)       # List: [1, <new>]
        left_node.right.info = 3

        self.assertEqual(self.dll.pop_left(), 1)
        self.assertEqual(self.dll.pop_left(), 3)

    def test_insert_node_in_middle(self):
        """Test inserting a node between two existing nodes."""
        self.dll.insert_right(1)              # List: [1]
        self.dll.insert_right(3)              # List: [1, 3]
        first = self.dll.head.right           # node holding 1
        self.dll.insert_node(first)           # List: [1, <new>, 3]
        first.right.info = 2

        self.assertEqual(self.dll.pop_left(), 1)
        self.assertEqual(self.dll.pop_left(), 2)
        self.assertEqual(self.dll.pop_left(), 3)

    def test_insert_node_at_tail_extends_list(self):
        """Test that inserting to the right of the tail node extends the list,
        and that the new tail is reachable from pop_right too."""
        self.dll.insert_right(1)
        self.dll.insert_right(2)              # List: [1, 2]
        tail = self.dll.head.left             # node holding 2
        self.dll.insert_node(tail)            # List: [1, 2, <new>]
        tail.right.info = 3

        self.assertEqual(self.dll.pop_right(), 3)
        self.assertEqual(self.dll.pop_right(), 2)
        self.assertEqual(self.dll.pop_right(), 1)

    def test_insert_node_respects_pool_limit(self):
        """Test that insert_node draws from the same shared pool as every
        other insertion method, and raises once the pool is exhausted."""
        self.dll.insert_right(1)
        anchor = self.dll.head.right          # node holding 1

        for _ in range(self.capacity - 1):
            self.dll.insert_node(anchor)
            anchor = anchor.right             # walk to the node just inserted

        with self.assertRaisesRegex(Exception, "Memory Overflow: STORAGE POOL is empty"):
            self.dll.insert_node(anchor)

    def test_delete_node_raises_on_none(self):
        """Test that delete_node refuses to delete a None reference."""
        with self.assertRaises(Exception):
            self.dll.delete_node(None)

    def test_delete_node_in_middle(self):
        """Test deleting a given node directly from the middle of the list."""
        self.dll.insert_right(1)
        self.dll.insert_right(2)
        self.dll.insert_right(3)
        # List: [1, 2, 3]
        middle = self.dll.head.right.right    # node holding 2

        self.dll.delete_node(middle)          # deletes node 2 itself
        # List: [1, 3]

        self.assertEqual(self.dll.pop_left(), 1)
        self.assertEqual(self.dll.pop_left(), 3)

    def test_delete_node_updates_head(self):
        """Test that deleting the first real node correctly updates head-side traversal."""
        self.dll.insert_right(1)
        self.dll.insert_right(2)
        # List: [1, 2]
        first = self.dll.head.right           # node holding 1

        self.dll.delete_node(first)           # deletes node 1 itself
        # List: [2]

        self.assertEqual(self.dll.pop_left(), 2)
        with self.assertRaises(Exception):
            self.dll.pop_left()

    def test_delete_node_updates_tail(self):
        """Test that deleting the current tail node correctly updates tail tracking."""
        self.dll.insert_right(1)
        self.dll.insert_right(2)
        # List: [1, 2]
        tail = self.dll.head.left             # node holding 2

        self.dll.delete_node(tail)            # deletes node 2 itself
        # List: [1]

        self.assertEqual(self.dll.pop_right(), 1)
        with self.assertRaises(Exception):
            self.dll.pop_right()

    def test_delete_node_frees_pool_slot(self):
        """Test that deleting a node returns its slot to the pool for reuse."""
        self.dll.insert_right(1)
        node = self.dll.head.right
        self.dll.delete_node(node)  # frees the one slot used above

        # Should be able to fill the pool back up to full capacity again
        for i in range(self.capacity):
            self.dll.insert_right(i)

        with self.assertRaisesRegex(Exception, "Memory Overflow: STORAGE POOL is empty"):
            self.dll.insert_right(99)

    def test_clear(self):
        """Test clearing the linked list back to the pool in O(1) time."""
        self.dll.insert_right(100)
        self.dll.insert_right(200)

        self.dll.clear()

        # List should be empty now, from both ends
        with self.assertRaises(Exception):
            self.dll.pop_left()
        with self.assertRaises(Exception):
            self.dll.pop_right()

    def test_clear_returns_nodes_to_pool(self):
        """Test that clear() actually returns nodes to the pool (not just
        resets the list's own pointers) by refilling to full capacity after."""
        for i in range(self.capacity):
            self.dll.insert_right(i)

        self.dll.clear()

        # Pool should be fully available again
        for i in range(self.capacity):
            self.dll.insert_right(i)

        with self.assertRaisesRegex(Exception, "Memory Overflow: STORAGE POOL is empty"):
            self.dll.insert_right(99)

    def test_clear_on_empty_list(self):
        """Test that clearing an already-empty list is a safe no-op."""
        self.dll.clear()

        with self.assertRaises(Exception):
            self.dll.pop_left()

    def test_union_appends_list_b_to_self(self):
        """Test that union appends list_b's contents to the right end of
        self, preserving order."""
        list_b = DoublyLinkedList(storage_pool=self.pool)

        self.dll.insert_right("A")
        self.dll.insert_right("B")

        list_b.insert_right("C")
        list_b.insert_right("D")

        self.dll.union(list_b)

        self.assertEqual(self.dll.pop_left(), "A")
        self.assertEqual(self.dll.pop_left(), "B")
        self.assertEqual(self.dll.pop_left(), "C")
        self.assertEqual(self.dll.pop_left(), "D")

    def test_union_empties_list_b(self):
        """Test that list_b no longer holds its elements after being
        unioned into another list."""
        list_b = DoublyLinkedList(storage_pool=self.pool)
        list_b.insert_right(1)
        list_b.insert_right(2)

        self.dll.union(list_b)

        with self.assertRaises(Exception):
            list_b.pop_left()

    def test_union_with_none_is_noop(self):
        """Test that unioning None is a safe no-op, not an error."""
        self.dll.insert_right(1)

        self.dll.union(None)  # should not raise

        self.assertEqual(self.dll.pop_left(), 1)
        with self.assertRaises(Exception):
            self.dll.pop_left()

    def test_union_with_empty_list_b(self):
        """Test unioning an already-empty list_b into a populated self
        leaves self unaffected."""
        empty_list = DoublyLinkedList(storage_pool=self.pool)
        self.dll.insert_right(1)

        self.dll.union(empty_list)

        self.assertEqual(self.dll.pop_left(), 1)
        with self.assertRaises(Exception):
            self.dll.pop_left()

    def test_union_into_empty_self(self):
        """Test unioning a populated list_b into an empty self."""
        list_b = DoublyLinkedList(storage_pool=self.pool)
        list_b.insert_right(99)

        self.dll.union(list_b)

        self.assertEqual(self.dll.pop_left(), 99)

    def test_union_with_self_is_safe_noop(self):
        """Test that unioning a list with itself doesn't corrupt or leak its
        nodes. A splice-based union() that only guards against `None` (and
        not `list_b is self`) will silently lose every node: `head.right`/
        `head.left` end up pointing back to `head` as if empty, while the
        original nodes are never returned to the pool either — so the pool
        permanently loses that capacity. This test catches exactly that."""
        self.dll.insert_right("A")
        self.dll.insert_right("B")
        self.dll.insert_right("C")

        self.dll.union(self.dll)

        # The list must still report and yield its original contents...
        self.assertEqual(self.dll.pop_left(), "A")
        self.assertEqual(self.dll.pop_left(), "B")
        self.assertEqual(self.dll.pop_left(), "C")

        # ...and the pool must not have silently lost capacity: a full
        # refill afterward should reach exactly `capacity` before overflowing.
        for i in range(self.capacity):
            self.dll.insert_right(i)
        with self.assertRaisesRegex(Exception, "Memory Overflow: STORAGE POOL is empty"):
            self.dll.insert_right("overflow")

    def test_union_chain_of_three_lists(self):
        """Test that unioning can be chained: self absorbs list_b, then
        absorbs list_c, preserving the overall order across all three."""
        list_b = DoublyLinkedList(storage_pool=self.pool)
        list_c = DoublyLinkedList(storage_pool=self.pool)

        self.dll.insert_right("A")
        list_b.insert_right("B")
        list_c.insert_right("C")

        self.dll.union(list_b)
        self.dll.union(list_c)

        self.assertEqual(self.dll.pop_left(), "A")
        self.assertEqual(self.dll.pop_left(), "B")
        self.assertEqual(self.dll.pop_left(), "C")

    def test_list_b_is_reusable_after_union(self):
        """Test that list_b, once emptied by union, still works normally
        for further insertions (its own sentinel wasn't corrupted)."""
        list_b = DoublyLinkedList(storage_pool=self.pool)
        list_b.insert_right(1)

        self.dll.union(list_b)
        list_b.insert_right(99)  # list_b should behave like a fresh empty list

        self.assertEqual(list_b.pop_left(), 99)
        with self.assertRaises(Exception):
            list_b.pop_left()

    def test_interleaved_insert_node_and_delete_node(self):
        """Test a longer sequence mixing insert_node and delete_node at
        different positions, verifying the final order is correct."""
        self.dll.insert_right(1)              # [1]
        node_1 = self.dll.head.right

        self.dll.insert_node(node_1)          # [1, _]
        node_2 = node_1.right
        node_2.info = 2                       # [1, 2]

        self.dll.insert_node(node_2)          # [1, 2, _]
        node_4 = node_2.right
        node_4.info = 4                       # [1, 2, 4]

        self.dll.insert_node(node_1)          # [1, _, 2, 4]
        node_99 = node_1.right
        node_99.info = 99                     # [1, 99, 2, 4]

        self.dll.delete_node(node_99)         # deletes 99 itself -> [1, 2, 4]

        self.dll.insert_node(node_2)          # [1, 2, _, 4]
        node_3 = node_2.right
        node_3.info = 3                       # [1, 2, 3, 4]

        self.assertEqual(self.dll.pop_left(), 1)
        self.assertEqual(self.dll.pop_left(), 2)
        self.assertEqual(self.dll.pop_left(), 3)
        self.assertEqual(self.dll.pop_left(), 4)

    def test_delete_all_nodes_individually_then_refill(self):
        """Test deleting every node one at a time via delete_node, and that
        the pool correctly reclaims all of them for a full refill after."""
        for i in range(self.capacity):
            self.dll.insert_right(i)

        # delete every real node, always taking the current first one
        for _ in range(self.capacity):
            node = self.dll.head.right
            self.dll.delete_node(node)

        with self.assertRaises(Exception):
            self.dll.pop_left()

        # pool should be fully reclaimed: exactly `capacity` more fit
        for i in range(self.capacity):
            self.dll.insert_right(i)
        with self.assertRaisesRegex(Exception, "Memory Overflow: STORAGE POOL is empty"):
            self.dll.insert_right("overflow")

    def test_pool_slot_reused_after_delete_then_insert(self):
        """Test that a node freed by delete_node is actually available again
        for the very next insertion (LIFO reuse), not just conceptually free."""
        self.dll.insert_right(1)
        self.dll.insert_right(2)
        node_to_free = self.dll.head.right.right  # node holding 2
        self.dll.delete_node(node_to_free)

        # Immediately re-fill to exactly full capacity; if the freed slot
        # weren't actually returned, this would overflow one insert early.
        for i in range(self.capacity - 1):
            self.dll.insert_right(i)
        with self.assertRaisesRegex(Exception, "Memory Overflow: STORAGE POOL is empty"):
            self.dll.insert_right("overflow")

    def test_clear_then_union_then_reinsert(self):
        """Test a longer workflow: populate, clear, union in fresh data from
        another list, then continue inserting normally."""
        self.dll.insert_right("stale1")
        self.dll.insert_right("stale2")
        self.dll.clear()

        list_b = DoublyLinkedList(storage_pool=self.pool)
        list_b.insert_right("fresh1")
        list_b.insert_right("fresh2")

        self.dll.union(list_b)
        self.dll.insert_right("fresh3")

        self.assertEqual(self.dll.pop_left(), "fresh1")
        self.assertEqual(self.dll.pop_left(), "fresh2")
        self.assertEqual(self.dll.pop_left(), "fresh3")

    def test_capacity_overflow(self):
        """Test inserting beyond the storage pool constraint, accounting for
        the one node the list itself reserved as its head."""
        # Fill the pool's remaining capacity (5 data nodes, since 1 of the
        # 6 pool slots was already claimed by the list's head on construction)
        for i in range(self.capacity):
            self.dll.insert_right(i)

        # The next node request should throw a contextually accurate Memory Overflow exception
        with self.assertRaisesRegex(Exception, "Memory Overflow: STORAGE POOL is empty"):
            self.dll.insert_right(99)


if __name__ == '__main__':
    unittest.main()