#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <span>

template <std::size_t M>
// waiting + free_bytes = pop_bunch
class RingBuffer {
public:
    static_assert(M >= 1, "RingBuffer size must be >= 1");
    static_assert((M & (M - 1)) == 0, "RingBuffer size must be a power of 2");
    static_assert(std::atomic<std::size_t>::is_always_lock_free, "Hidden deadlock occurs");

    // Push a byte into queue
    bool push(std::uint8_t byte) noexcept {
        const std::size_t F = FRONT.load(std::memory_order_acquire);
        const std::size_t R = REAR.load(std::memory_order_relaxed);

        // Overflow
        if (R - F == M) {
            DROPPED.fetch_add(1, std::memory_order_relaxed);
            return false;
        }

        // Write a byte
        BUFFER[R & MASK] = byte;

        // Advance REAR
        REAR.store(R + 1, std::memory_order_release);
        return true;
    }

    // Push a bunch of bytes to the buffer
    bool push(std::span<const std::uint8_t> data) noexcept {
        const std::size_t F = FRONT.load(std::memory_order_acquire);
        std::size_t R = REAR.load(std::memory_order_relaxed);

        // Overflow
        if (data.size() > (M - (R - F))) {
            DROPPED.fetch_add(data.size(), std::memory_order_relaxed);
            return false;
        }

        // Write a bunch of bytes
        for (std::uint8_t byte : data) {
            BUFFER[R++ & MASK] = byte;
        }

        // Advance REAR
        REAR.store(R, std::memory_order_release);

        return true;
    }

    // Return waiting bytes
    std::span<const std::uint8_t> waiting() const noexcept {

    }

    [[nodiscard]] bool free_bytes(std::size_t n) noexcept {
        const std::size_t F = FRONT.load(std::memory_order_relaxed);
        const std::size_t R = REAR.load(std::memory_order_acquire);

        // Underflow
        if (n > R - F) return false;

        // Advance FRONT
        FRONT.store(F + n, std::memory_order_release);
        return true;
    }

    // Delete from queue
    bool pop(std::uint8_t& byte) noexcept {
        const std::size_t F = FRONT.load(std::memory_order_relaxed);
        const std::size_t R = REAR.load(std::memory_order_acquire);

        // Underflow
        if (F == R) return false;

        // Read a byte
        byte = BUFFER[F & MASK];

        // Advance FRONT
        FRONT.store(F + 1, std::memory_order_release);
        return true;
    }

    std::size_t size() const noexcept {
        const std::size_t F = FRONT.load(std::memory_order_acquire);
        const std::size_t R = REAR.load(std::memory_order_acquire);
        return R - F;
    }

    std::size_t dropped() const noexcept {
        return DROPPED.load(std::memory_order_relaxed);
    }

private:
    static constexpr std::size_t MASK = M - 1;
    std::atomic<std::size_t> FRONT = 0;
    std::atomic<std::size_t> REAR = 0;
    std::atomic<std::size_t> DROPPED = 0;
    std::array<std::uint8_t, M> BUFFER{};
};
