#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <span>

#include "crc16.hpp"

// size of *XXXX\r\n
inline constexpr std::size_t SEAL_SIZE = 7;

// Seal in place
// The body is already in buf, add *XXXX\r\n after it
// buffer size must be body_len + 7
[[nodiscard]] constexpr std::string_view seal(std::span<char> buf, std::size_t len) noexcept {
    if (len + 7 > buf.size()) return {};

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

// Write the body of the line to the out
// body and crc point into the caller's buffer, 
// they're only valid as long as the buffer is unchanged
[[nodiscard]] constexpr bool unseal(std::string_view line, std::span<char> out) noexcept {
    //  Get rid of \r\n
    if (line.ends_with('\n')) line.remove_suffix(1);
    if (line.ends_with('\r')) line.remove_suffix(1);

    // Check the length and body
    if (line.size() < 5 || line[line.size() - 5] != '*') return false;
    
    // Split body and crc
    const std::string_view body = line.substr(0, line.size() - 5);
    const std::string_view crc = line.substr(line.size() - 4);

    // Forbid "\r" and "\n" in the body
    if (body.contains('\r') || body.contains('\n')) return false;

    // Check crc
    const auto hex = crc_to_hex(crc16(body));
    if (crc != std::string_view{hex.data(), hex.size()}) return false;

    // ASCII check
    for (char c : body) {
        if (static_cast<unsigned char>(c) > 127) return false;
    }

    return true;
}
