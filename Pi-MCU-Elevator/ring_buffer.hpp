#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

template <std::size_t Max>
class RingBuffer {
public:
    static_assert(Max >= 2, "RingBuffer size must be >= 2");
    static_assert((Max & (Max - 1)) == 0, "RingBuffer size must be a power of 2");

    // Insert into queue
    bool push(std::uint8_t byte) noexcept {
        // If Rear == Max - 1: Rear = 0, else Rear = Rear + 1
        std::size_t next = (Rear == Max - 1) ? 0 : Rear + 1;

        // If next Rear == Front: Overflow
        if (next == Front) return false;

        // Write a byte first
        buffer[Rear] = byte;

        // Then move Rear
        Rear = next;
        return true;
    }

    // Delete from queue
    bool pop(std::uint8_t& byte) noexcept {
        // Underflow
        if (Front == Rear) return false;

        // Y = buffer[Front]
        byte = buffer[Front];

        // If Front == Max - 1: Front = 0, else Front = Front + 1
        Front = (Front == Max - 1) ? 0 : Front + 1; 
        return true;
    }

private:
    std::size_t Front = 0;
    std::size_t Rear = 0;
    std::array<std::uint8_t, Max> buffer{};
};
