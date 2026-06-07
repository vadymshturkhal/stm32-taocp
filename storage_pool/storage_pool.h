#ifndef STORAGE_POOL_H
#define STORAGE_POOL_H

#include <stdint.h>

typedef struct {
    void* avail;
    uint32_t node_size;  // there might be some Storage Pools with different node sizes
 } Storage_Pool;

Storage_Pool* create_storage_pool(void* memory, uint32_t node_size, uint32_t storage_size);
void* storage_pool_pop(Storage_Pool* storage_pool);
bool storage_pool_push(Storage_Pool* storage_pool, void* node);
bool storage_pool_add_slice(Storage_Pool* storage_pool, void* head, void* tail);

#endif
