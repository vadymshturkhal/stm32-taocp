#pragma once
#include <cstddef>
#include <cstdint>

// The first number in every message
enum class MsgType : std::uint8_t {
    SYN     = 0,
    SYNACK  = 1,
    ACK     = 2,
    RST     = 3,
    DATA    = 4,
};

inline constexpr std::uint32_t PROTOCOL_VERSION   = 1;

// RingBuffer<RX_BUFFER_SIZE> sent as a <window>
inline constexpr std::size_t   RX_BUFFER_SIZE     = 1024;

// Back to LISTEN after TIMEOUT expires without receiving any ACK
inline constexpr std::uint32_t SESSION_TIMEOUT_MS = 4000;
