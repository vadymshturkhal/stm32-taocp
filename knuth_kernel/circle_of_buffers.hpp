#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cassert>

/*
A circle of buffers from TAOCP Vol.1 (1.4.4) with atomics and asserts
N = total number of buffers.
n = current number of red buffers.

NOTE: after start_operation() only operation_is_completed() can free the buffer

Small changes:
- A1 and B2 return nullptr
- A3 moved to RELEASE for returning the same yellow slot every time until next RELEASE
- Assert in RELEASE and start_operation() and operation_is_completed()
*/
template <typename T, std::size_t N>
class CircleOfBuffers {
public:
    static_assert(N >= 1, "CircleOfBuffers size must be >= 1.");
    static_assert((N & (N - 1)) == 0, "CircleOfBuffers size must be a power of 2");

    // Algorithm A [ASSIGN]
    [[nodiscard]] T* assign() noexcept {
        // [A1. Wait for n < N]
        // If n == N, stall the program  until n < N.
        // If n == N, no buffers are ready to be assigned.
        if (n.load(std::memory_order_acquire) == N) return nullptr;

        // [A2. CURRENT <- NEXTG]
        std::size_t CURRENT = NEXTG;

        return &CIRCLE_OF_BUFFERS[CURRENT];
    }

    // Algorithm R [RELEASE]
    void release() noexcept {
        assert(n.load(std::memory_order_relaxed) < N);  // Release without a successful Assign
        
        // [R1. Increase n]
        n.fetch_add(1, std::memory_order_release);

        // [A3. Advance NEXTG]
        NEXTG = (NEXTG + 1) & MASK;
    }

    // Algorithm B [Buffer control]
    [[nodiscard]] T* start_operation() noexcept {
        // [B1 is a kernel routine]
        // [B2 return to kernel]
        // No buffers are red, no I/O device is ready for another operation
        if (DEVICE_BUFFER.load(std::memory_order_acquire)) return nullptr;  // device is busy
        if (n.load(std::memory_order_acquire) == 0) return nullptr;

        // [B3. Transmission between the buffer area designed by NEXTR and I/O device]
        // [B4. Kernel loop]
        T* buffer = &CIRCLE_OF_BUFFERS[NEXTR];
        DEVICE_BUFFER.store(buffer, std::memory_order_relaxed);
        return buffer;
    }

    // The device refused the buffer from start_operation
    // start_operation() will return the same buffer after this
    void cancel_operation() noexcept {
        assert(DEVICE_BUFFER.load(std::memory_order_relaxed) != nullptr);
        DEVICE_BUFFER.store(nullptr, std::memory_order_release);
    }

    void operation_is_completed() noexcept {
        assert(DEVICE_BUFFER.load(std::memory_order_relaxed) != nullptr);
    
        // [B5. Advance NEXTR]
        NEXTR = (NEXTR + 1) & MASK;

        // [B6. Decrease n]
        n.fetch_sub(1, std::memory_order_release);

        // Clear the device
        DEVICE_BUFFER.store(nullptr, std::memory_order_release);
    }

private:
    static constexpr std::size_t MASK = N - 1;
    std::size_t NEXTR = 0;
    std::size_t NEXTG = 0;
    std::atomic<T*> DEVICE_BUFFER = nullptr;
    std::atomic<std::size_t> n = 0;
    std::array<T, N> CIRCLE_OF_BUFFERS{};
};
