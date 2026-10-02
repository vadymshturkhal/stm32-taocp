#pragma once
#include <cstdint>
#include <span>
#include <string_view>

// One byte for CRC
constexpr std::uint16_t crc16_update(std::uint16_t crc, std::uint8_t byte) noexcept {
    // 2. For each byte, XOR it into the top 8 bits of crc
    crc ^= static_cast<std::uint16_t>(byte << 8);

    // 3. Repeat 8 times
    for (int bit = 0; bit < 8; ++bit) {
        // look at the top bit
        bool top_bit = crc & 0x8000;
        
        // shift crc left by 1
        crc <<= 1;

        // and if the top is 1: XOR the polynomial
        if (top_bit) crc ^= 0x1021;
    }

    return crc;
}

// Raw bytes
constexpr std::uint16_t crc16(std::span<const std::uint8_t> data) noexcept {
    // 1. Start with crc = 0xFFFF
    std::uint16_t crc = 0xFFFF;

    // 2-3.
    for (std::uint8_t byte : data) {
        crc = crc16_update(crc, byte);
    }

    return crc;
}

// Text
constexpr std::uint16_t crc16(std::string_view text) noexcept {
    // same as a regular crc16 with a static_cast to uint8_t
    std::uint16_t crc = 0xFFFF;

    // 2-3.
    for (char c : text) {
        crc = crc16_update(crc, static_cast<std::uint8_t>(c));
    }

    return crc;
}
