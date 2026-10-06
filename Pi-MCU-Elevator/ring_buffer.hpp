#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

template <std::size_t Max>
class RingBuffer {
public:
    static_assert(Max >= 1, "RingBuffer size must be >= 1");
    static_assert((Max & (Max - 1)) == 0, "RingBuffer size must be a power of 2");

    // Insert into queue
    bool push(std::uint8_t byte) noexcept {
        // Overflow
        if (Rear - Front == Max) return false;

        // Write a byte
        buffer[Rear & MASK] = byte;

        // Advance Rear
        ++Rear;
        return true;
    }

    // Delete from queue
    bool pop(std::uint8_t& byte) noexcept {
        // Underflow
        if (Front == Rear) return false;

        // Read a byte
        byte = buffer[Front & MASK];

        // Advance Front
        ++Front;
        return true;
    }

private:
    static constexpr std::size_t MASK = Max - 1;
    std::size_t Front = 0;
    std::size_t Rear = 0;
    std::array<std::uint8_t, Max> buffer{};
};
