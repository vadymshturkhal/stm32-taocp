#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "c_queue.h"


// Stats with 128 nodes, 128 Enqueue and 128 Dequeue using Bump Allocator (balloc)
// cycles_cold = [2747], cycles_warm = [2653], size = 428 bytes

// FFI Allocator Definitions
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* ptr);

uint8_t c_perform_queue_operations_integrated(uint16_t max_nodes) {
    if (max_nodes == 0) return 0;

    void* c_queue_memory = asm_balloc(max_nodes * sizeof(Node) + sizeof(Queue));
    if (c_queue_memory == NULL) return 0;

    Queue* queue = c_create_queue(c_queue_memory, max_nodes);

    // =========================================================================
    // ENQUEUE HOT LOOP: L0 Caching + Deferred NULL Linkage
    // =========================================================================

    // 1. L0 Caching: Force GCC to pin these into ARM registers
    Node* avail = queue->avail;
    Node** rear = queue->rear;

    #pragma GCC unroll 4
    for (uint32_t i = max_nodes; i > 0; i--) {
        if (avail == NULL) {
            asm_balloc_free(c_queue_memory);
            return 0;
        }

        Node* P = avail;
        avail = avail->link;       // Advance Avail register

        P->info = i;

        // 2. THE SAFETY TAX EVASION:
        // We deliberately do NOT execute P->link = NULL; here.
        // We leave the memory dirty to save an instruction cycle.

        *rear = P;                 // Torvalds Linkage
        rear = &P->link;           // Advance Rear register
    }

    // 3. CAPPING THE MATRIX: Apply the Deferred NULL exactly once
    *rear = NULL;

    // 4. FLUSH L0 CACHE TO SRAM
    queue->avail = avail;
    queue->rear = rear;


    // =========================================================================
    // DEQUEUE HOT LOOP: Redundant Load Elimination
    // =========================================================================

    // Pin front into a register. 'avail' is already pinned from the enqueue loop!
    Node* front = queue->front;

    #pragma GCC unroll 4
    for (uint32_t i = max_nodes; i > 0; i--) {
        if (front == NULL) {
            asm_balloc_free(c_queue_memory);
            return 0;
        }

        Node* P = front;
        front = P->link;           // Advance Front register

        P->link = avail;           // Return to Avail pool
        avail = P;                 // Update Avail head register
    }

    // FLUSH L0 CACHE TO SRAM AND FIX REAR POINTER
    if (front == NULL) {
        queue->rear = &queue->front;
    }
    queue->front = front;
    queue->avail = avail;

    // =========================================================================

    asm_balloc_free(c_queue_memory);
    return 1;
}
