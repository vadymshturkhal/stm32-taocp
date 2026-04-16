#include <stdint.h>
#include <stddef.h>
#include <stdlib.h> // Pulling in the OS memory manager
#include "pairs.h"

// Standard Linked List Node
typedef struct StandardNode {
    uint32_t val;
    struct StandardNode* next;
} StandardNode;

uint8_t leetcode_regular_topological_sort(uint8_t n, Pair* input_pairs, uint8_t input_pairs_len, uint32_t* output) {

    // 1. The Standard Allocations (Dedicated arrays for everything)
    uint32_t* in_degree = (uint32_t*)calloc(n + 1, sizeof(uint32_t));
    StandardNode** adj_list = (StandardNode**)calloc(n + 1, sizeof(StandardNode*));
    uint32_t* queue = (uint32_t*)malloc((n + 1) * sizeof(uint32_t));

    // Safety check
    if (!in_degree || !adj_list || !queue) {
        free(in_degree); free(adj_list); free(queue);
        return 0;
    }

    // 2. Build the Graph (Calling malloc for EVERY edge)
    for (uint32_t i = 0; i < input_pairs_len; i++) {
        uint32_t u = input_pairs[i].j;
        uint32_t v = input_pairs[i].k;

        in_degree[v]++;

        // HEAP FRAGMENTATION INCOMING
        StandardNode* new_node = (StandardNode*)malloc(sizeof(StandardNode));
        if (!new_node) {
            // A nightmare cleanup scenario we will ignore for brevity
            return 0;
        }
        new_node->val = v;
        new_node->next = adj_list[u];
        adj_list[u] = new_node;
    }

    // 3. Initialize the standard Queue
    uint32_t head = 0;
    uint32_t tail = 0;

    for (uint32_t i = 1; i <= n; i++) {
        if (in_degree[i] == 0) {
            queue[tail++] = i;
        }
    }

    // 4. The Engine
    uint32_t processed = 0;
    while (head < tail) {
        uint32_t curr = queue[head++];
        output[processed++] = curr;

        StandardNode* neighbor = adj_list[curr];
        while (neighbor != NULL) {
            in_degree[neighbor->val]--;
            if (in_degree[neighbor->val] == 0) {
                queue[tail++] = neighbor->val;
            }
            neighbor = neighbor->next;
        }
    }

    // 5. The Standard C Cleanup (The O(V+E) Freeing Loop)
    for (uint32_t i = 1; i <= n; i++) {
        StandardNode* curr = adj_list[i];
        while (curr != NULL) {
            StandardNode* temp = curr;
            curr = curr->next;
            free(temp); // Sledgehammering the heap block by block
        }
    }

    free(in_degree);
    free(adj_list);
    free(queue);

    return (processed == n) ? 1 : 0;
}
