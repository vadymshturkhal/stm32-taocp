#include <stdint.h>
#include <stdbool.h>
#include "c_circular_list.h"

extern void* asm_balloc(uint32_t size);
extern void  asm_balloc_free(void* ptr);

#define TEST_ASSERT(expr) \
    if (!(expr)) { \
        __disable_irq(); \
        while(1) { __asm("nop"); } \
    }

#define MAX_TEST_NODES 5
#define MEMORY_BLOCK_SIZE (sizeof(CircularList) + (MAX_TEST_NODES * sizeof(CircularNode)))

// --- ALLOCATION MACROS ---
// Now uses your custom ASM allocator instead of local stack arrays
#define CREATE_TEST_LIST(name) \
    void* mem_##name = asm_balloc(MEMORY_BLOCK_SIZE); \
    TEST_ASSERT(mem_##name != NULL); /* Ensure allocation succeeded */ \
    CircularList* name = c_create_circular_list(mem_##name, MAX_TEST_NODES)

#define FREE_TEST_LIST(name) \
	asm_balloc_free(mem_##name)

#define TEST_ASSERT(expr) \
    if (!(expr)) { \
        return false; \
    }

bool test_insert_left(void) {
	CREATE_TEST_LIST(list);

    // 1. Insert into empty list
    bool success = circular_list_insert_left(list, 10);
    TEST_ASSERT(success == true);
    TEST_ASSERT(list->ptr != NULL);
    TEST_ASSERT(list->ptr->info == 10);
    TEST_ASSERT(list->ptr->link == list->ptr);

    // 2. Insert another
    success = circular_list_insert_left(list, 20);
    TEST_ASSERT(success == true);
    TEST_ASSERT(list->ptr->info == 10);
    TEST_ASSERT(list->ptr->link->info == 20);

    // 3. Test Overflow
    circular_list_insert_left(list, 30);
    circular_list_insert_left(list, 40);
    circular_list_insert_left(list, 50);

    success = circular_list_insert_left(list, 60);
    TEST_ASSERT(success == false);

    FREE_TEST_LIST(list);
	return true; // Test passed!
}

bool test_insert_right(void) {
    CREATE_TEST_LIST(list);

    bool success = circular_list_insert_right(list, 100);
    TEST_ASSERT(success == true);
    TEST_ASSERT(list->ptr->info == 100);

    success = circular_list_insert_right(list, 200);
    TEST_ASSERT(success == true);
    TEST_ASSERT(list->ptr->info == 200);
    TEST_ASSERT(list->ptr->link->info == 100);

    FREE_TEST_LIST(list);
	return true; // Test passed!
}

bool test_pop(void) {
    CREATE_TEST_LIST(list);
    bool pop_success = true;

    // 1. Underflow check
    uint32_t val = circular_list_pop(&pop_success, list);
    TEST_ASSERT(pop_success == false);

    // 2. FIFO behavior
    circular_list_insert_right(list, 11);
    circular_list_insert_right(list, 22);
    circular_list_insert_right(list, 33);

    pop_success = true;
    val = circular_list_pop(&pop_success, list);
    TEST_ASSERT(pop_success == true && val == 11);

    val = circular_list_pop(&pop_success, list);
    TEST_ASSERT(pop_success == true && val == 22);

    // 3. Pop the very last item
    val = circular_list_pop(&pop_success, list);
    TEST_ASSERT(pop_success == true && val == 33);
    TEST_ASSERT(list->ptr == NULL);

    FREE_TEST_LIST(list);
	return true;
}

bool test_clear(void) {
    CREATE_TEST_LIST(list);

    circular_list_clear(list);
    TEST_ASSERT(list->ptr == NULL);

    for (int i = 0; i < MAX_TEST_NODES; i++) {
        circular_list_insert_right(list, i);
    }

    circular_list_clear(list);
    TEST_ASSERT(list->ptr == NULL);

    bool success = circular_list_insert_right(list, 99);
    TEST_ASSERT(success == true);

    FREE_TEST_LIST(list);
	return true;
}

bool test_union(void) {
    CREATE_TEST_LIST(list_a);
    CREATE_TEST_LIST(list_b);

    circular_list_insert_right(list_a, 1);
    circular_list_insert_right(list_a, 2);

    circular_list_insert_right(list_b, 3);
    circular_list_insert_right(list_b, 4);

    circular_list_union(list_a, list_b);

    TEST_ASSERT(list_b->ptr == NULL);

    bool pop_ok;
    TEST_ASSERT(circular_list_pop(&pop_ok, list_a) == 1);
    TEST_ASSERT(circular_list_pop(&pop_ok, list_a) == 2);
    TEST_ASSERT(circular_list_pop(&pop_ok, list_a) == 3);
    TEST_ASSERT(circular_list_pop(&pop_ok, list_a) == 4);

    circular_list_insert_right(list_a, 99);
    circular_list_union(list_a, list_b);
    TEST_ASSERT(circular_list_pop(&pop_ok, list_a) == 99);

    // Important: Free both memory blocks!
    FREE_TEST_LIST(list_a);
    FREE_TEST_LIST(list_b);
	return true; // Test passed!
}

bool run_all_tests(void) {
    if (!test_insert_left())  return false;
    if (!test_insert_right()) return false;
    if (!test_pop())          return false;
    if (!test_clear())        return false;
    if (!test_union())        return false;

    // If we made it through all functions without returning false, everything passed!
    return true;
}
