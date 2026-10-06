#pragma once
#include <vector>
#include <atomic>
#include <cstddef>
#include <cstdint>

class RingBuffer {
public:
    RingBuffer(size_t capacity): Max(capacity + 1), Front(0), Rear(0), buffer(capacity + 1) {}

    // insert into queue
    bool push(std::uint8_t byte) noexcept {
        // If Rear == Max: Rear = 0, else Rear = Rear + 1 (?)
        std::size_t next = (Rear == Max - 1) ? 0 : Rear + 1;

        // If Rear == Front: Overflow (?)
        if (next == Front) return false;

        Rear = next;

        buffer[Rear] = byte;
        return true;
    }

    // Delete from queue
    bool pop(std::uint8_t& byte) noexcept {
        // Underflow
        if (Front == Rear) return false;

        // If Front == Max: Front = 0, else Front = Front + 1
        Front = (Front == Max - 1) ? 0 : Front + 1; 

        // Y = buffer[Front]
        byte = buffer[Front];
        return true;
    }

private:
    const size_t Max;
    size_t Front;
    size_t Rear;
    std::vector<std::uint8_t> buffer;
};
