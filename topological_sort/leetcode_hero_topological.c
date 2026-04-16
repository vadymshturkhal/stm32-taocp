#include <stdint.h>
#include <stddef.h>
#include <stdlib.h> // Pulling in the OS memory manager
#include "pairs.h"

uint8_t leetcode_hero_topological_sort(uint8_t n, Pair* input_pairs, uint8_t input_pairs_len, uint32_t* output) {

    // 1. The Standard Allocations
    uint32_t* in_degree = (uint32_t*)calloc(n + 1, sizeof(uint32_t));
    uint32_t** graph = (uint32_t**)malloc((n + 1) * sizeof(uint32_t*));
    uint32_t* graph_col_size = (uint32_t*)calloc(n + 1, sizeof(uint32_t));
    uint32_t* queue = (uint32_t*)malloc((n + 1) * sizeof(uint32_t));

    if (!in_degree || !graph || !graph_col_size || !queue) {
        // Standard C cleanup nightmare if initial allocs fail
        free(in_degree); free(graph); free(graph_col_size); free(queue);
        return 0;
    }

    for (uint32_t i = 0; i <= n; i++) {
        graph[i] = NULL; // Initialize pointers safely
    }

    // Pass 1: Count the outgoing edges for each node
    for (uint32_t i = 0; i < input_pairs_len; i++) {
        uint32_t u = input_pairs[i].j;
        uint32_t v = input_pairs[i].k;
        graph_col_size[u]++;
        in_degree[v]++;
    }

    // Pass 2: Allocate EXACTLY enough contiguous memory for each node's neighbors
    for (uint32_t i = 1; i <= n; i++) {
        if (graph_col_size[i] > 0) {
            graph[i] = (uint32_t*)malloc(graph_col_size[i] * sizeof(uint32_t));
            if (!graph[i]) goto leetcode_cleanup; // If the heap shatters midway...
        }
        graph_col_size[i] = 0; // Reset this so we can use it as an insertion index next
    }

    // Pass 3: Actually map the edges into the newly allocated arrays
    for (uint32_t i = 0; i < input_pairs_len; i++) {
        uint32_t u = input_pairs[i].j;
        uint32_t v = input_pairs[i].k;
        graph[u][graph_col_size[u]++] = v;
    }

    // Initialize the Queue
    uint32_t head = 0;
    uint32_t tail = 0;

    for (uint32_t i = 1; i <= n; i++) {
        if (in_degree[i] == 0) {
            queue[tail++] = i;
        }
    }

    // The Execution Engine (Standard Kahn's Algorithm)
    uint32_t processed = 0;
    while (head < tail) {
        uint32_t curr = queue[head++];
        output[processed++] = curr; // Writing directly to your pre-allocated array!

        for (uint32_t i = 0; i < graph_col_size[curr]; i++) {
            uint32_t next = graph[curr][i];
            in_degree[next]--;
            if (in_degree[next] == 0) {
                queue[tail++] = next;
            }
        }
    }

    uint8_t status = (processed == n) ? 1 : 0;

leetcode_cleanup:
    // The Massive Cleanup Phase
    for (uint32_t i = 0; i <= n; i++) {
        if (graph[i]) free(graph[i]);
    }
    free(graph);
    free(graph_col_size);
    free(in_degree);
    free(queue);

    return status;
}
