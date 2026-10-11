#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <span>

#include "crc16.hpp"

// size of XXXX
inline constexpr std::size_t CRC_SIZE = 4;

// size of *XXXX
inline constexpr std::size_t STAR_CRC_SIZE = CRC_SIZE + 1;

// size of *XXXX\r\n
inline constexpr std::size_t SEAL_SIZE = STAR_CRC_SIZE + 2;

// Seal in place
// The body is already in buf, add *XXXX\r\n after it
// buffer size must be body_len + SEAL_SIZE
[[nodiscard]] constexpr std::string_view seal(std::span<char> buf, std::size_t len) noexcept {
    if (len + SEAL_SIZE > buf.size()) return {};

    const std::string_view body = std::string_view{buf.data(), len};

    // Check the string for correctness
    for (char c : body) {
        if (c == '\r' || c =='\n' || static_cast<unsigned char>(c) > 127) return {};
    }

    // Calculate crc
    std::uint16_t crc = crc16(body);

    // Add * to the string
    buf[len++] = '*';

    // Add crc hex to the string
    for (char c : crc_to_hex(crc)) {
        buf[len++] = c;
    }

    // Add \r\n
    buf[len++] = '\r';
    buf[len++] = '\n';

    return {buf.data(), len};
}

// Return the body of a sealed line or std::nullopt
// body point into line's buffer, and is valid as long as the buffer is unchanged
[[nodiscard]] constexpr std::optional<std::string_view> unseal(std::string_view line) noexcept {
    //  Get rid of \r\n
    if (line.ends_with('\n')) line.remove_suffix(1);
    if (line.ends_with('\r')) line.remove_suffix(1);

    // Check the length and body
    if (line.size() < STAR_CRC_SIZE || line[line.size() - STAR_CRC_SIZE] != '*') return std::nullopt;
    
    // Split body and crc
    const std::string_view body = line.substr(0, line.size() - STAR_CRC_SIZE);
    const std::string_view crc = line.substr(line.size() - CRC_SIZE);

    // Forbid "\r" and "\n" in the body
    if (body.contains('\r') || body.contains('\n')) return std::nullopt;

    // Check crc
    const auto hex = crc_to_hex(crc16(body));
    if (crc != std::string_view{hex.data(), hex.size()}) return std::nullopt;

    // ASCII check
    for (char c : body) {
        if (static_cast<unsigned char>(c) > 127) return std::nullopt;
    }

    return body;
}
