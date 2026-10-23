// storage_pool.cpp - storage_pool.c ported 1:1
#include <cstdint>
#include "cstyle.hpp"

namespace cstyle {

StoragePool* create_storage_pool(void* memory, std::uint32_t node_size, std::uint32_t storage_size) {
    // Every node must begin with a link pointer as its first field
    if (node_size == 0 || storage_size == 0) return nullptr;

    auto* storage_pool = static_cast<StoragePool*>(memory);
    storage_pool->node_size = node_size;
    storage_pool->avail = nullptr;
    storage_pool->size = storage_size;

    // Nodes' starting memory address
    auto* memory_cursor = reinterpret_cast<std::uint8_t*>(storage_pool + 1);

    void* temp_avail = nullptr;
    for (std::uint32_t i = storage_size; i > 0; i--) {
        void* current_node = memory_cursor;
        memory_cursor += node_size;

        // Link it to the previous avail node
        *static_cast<void**>(current_node) = temp_avail;
        temp_avail = current_node;
    }

    storage_pool->avail = temp_avail;
    return storage_pool;
}

void* storage_pool_pop(StoragePool* storage_pool) {
    if (storage_pool->avail == nullptr) return nullptr;  // Overflow

    void* P = storage_pool->avail;
    storage_pool->avail = *static_cast<void**>(P);
    storage_pool->size -= 1;
    return P;
}

bool storage_pool_push(StoragePool* storage_pool, void* node) {
    if (node == nullptr) return false;

    *static_cast<void**>(node) = storage_pool->avail;
    storage_pool->avail = node;
    storage_pool->size += 1;
    return true;
}

}  // namespace cstyle
