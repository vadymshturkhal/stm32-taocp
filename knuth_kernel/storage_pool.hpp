#pragma once
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>

#include "wait_list.hpp"

// A pool of nodes created at run time (TAOCP 2.2.3: the AVAIL list).
// One malloc in the constructor, every node constructed once there; after that pop and
// push only relink. A free node keeps the AVAIL link in WaitNode::left (Knuth's
// LINK field); its right still points to itself, so is_held() is false.
template <std::derived_from<WaitNode> T>
class StoragePool {
    static_assert(alignof(T) <= alignof(std::max_align_t), "malloc does not align T");

public:
    // malloc once, construct every T once, link all into AVAIL.
    // No exceptions: if count is 0 or too big, or malloc fails, the pool is
    // empty and valid() is false (check it at startup).
    explicit StoragePool(std::size_t count) noexcept {
        if (count == 0 || count > SIZE_MAX / sizeof(T)) return;

        memory = static_cast<T*>(std::malloc(sizeof(T) * count));
        if (memory == nullptr) return;
        size = count;

        for (std::size_t i = 0; i < count; ++i) {
            T* node = new (memory + i) T();     // the only construction
            node->left = avail;                 // LINK(node) <- AVAIL
            avail = node;                       // AVAIL <- node
        }
    }

    StoragePool(const StoragePool&) = delete;
    StoragePool& operator=(const StoragePool&) = delete;

    // The memory lives as long as the program; this matters only for the tests on the PC.
    ~StoragePool() {
        for (std::size_t i = 0; i < size; ++i) memory[i].~T();
        std::free(memory);
    }

    [[nodiscard]] bool valid() const noexcept { return memory != nullptr; }

    // A free node, or nullptr when all are in use (overflow).
    // It keeps the fields of its previous user: the caller sets them.
    [[nodiscard]] T* pop() noexcept {
        T* node = avail;
        if (node == nullptr) return nullptr;
        avail = static_cast<T*>(node->left);    // every node on AVAIL is a T
        node->left = node;                      // back to "in no list"
        return node;
    }

    // The node must come from this pool, be in use (popped) and be in no list.
    // In use and out of every list, a node points to itself both ways (pop,
    // WaitList::delete_node); on AVAIL its left is the next free node or nullptr,
    // never itself. That catches a second push of the same node: without the check,
    // pushing the AVAIL head again links it to itself, and every pop after that
    // returns the same object. Release builds refuse the push instead (no change).
    void push(T& node) noexcept {
        assert(owns(node) && "node is not from this pool");
        assert(!node.is_held() && "node is still in a list");
        assert(!is_free(node) && "node is already free: pushed twice");
        if (!owns(node) || node.is_held() || is_free(node)) { ++refused_; return; }

        node.left = avail;
        avail = &node;
    }

    [[nodiscard]] bool owns(const T& node) const noexcept {
        return memory != nullptr && &node >= memory && &node < memory + size;
    }

    // Pushes refused (another pool's node, a held one, a free one). Release builds
    // only (debug builds assert first); 0 in a correct program.
    [[nodiscard]] std::uint32_t refused() const noexcept { return refused_; }

    // On AVAIL: not in a list (right points to itself) but linked to the next free node
    [[nodiscard]] bool is_free(const T& node) const noexcept {
        return owns(node) && node.is_on_avail();
    }

private:
    T* memory = nullptr;        // count constructed T's, from malloc
    T* avail = nullptr;         // the first free node
    std::size_t size = 0;
    std::uint32_t refused_ = 0;     // see refused()
};
