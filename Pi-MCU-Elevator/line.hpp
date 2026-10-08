#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

#include "crc16.hpp"

constexpr std::array<char, 4> crc_to_hex(std::uint16_t crc) noexcept {
    constexpr std::string_view DIGITS = "0123456789ABCDEF";

    // & 0xF keeps the low 4 bits after a shift
    return {
        DIGITS[(crc >> 12) & 0xF],
        DIGITS[(crc >> 8)  & 0xF],
        DIGITS[(crc >> 4)  & 0xF],
        DIGITS[(crc       & 0xF)],
    };
}

// Return the body of the line or std::nullopt if line is damaged
constexpr std::optional<std::string_view> unseal(std::string_view line) noexcept {
    //  Get rid of \r\n
    if (line.ends_with('\n')) line.remove_suffix(1);
    if (line.ends_with('\r')) line.remove_suffix(1);

    // Check the length and body
    if (line.size() < 5 || line[line.size() - 5] != '*') return std::nullopt;
    
    // Split body and crc
    const std::string_view body = line.substr(0, line.size() - 5);
    const std::string_view crc = line.substr(line.size() - 4);

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
