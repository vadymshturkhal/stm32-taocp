#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

// Copy each byte to the internal buffer
// body can contain \r
// \n is the separator between lines or the last char in the line
template <std::size_t N>
class LineReader {
    static_assert(N >= 1, "LineReader needs at least one character for length");

public:
    bool feed(std::uint8_t byte) noexcept {
        if (ready) {
            len = 0;
            ready = false;
        }

        const char c = static_cast<char>(byte);

        // Overflowed line
        if (c == '\n' && overflow) {
            ++dropped_too_long;
            overflow = false;
            len = 0;
            return false;
        }

        // Empty line
        if (c == '\n' && len == 0) {
            return false;
        }

        // End of the line
        if (c == '\n') {
            ready = true;
            return true;
        }

        // Skip the byte
        if (overflow) {
            return false;
        }

        if (len == N) { 
            overflow = true;
        } else {
            buf[len++] = c;
        }

        return false;
    }

    std::string_view line() const noexcept { return {buf.data(), len}; }

    // Return dropped bytes quantity
    std::size_t dropped() const noexcept { return dropped_too_long; }

    // The longest line with trailing \r
    static constexpr std::size_t capacity() noexcept { return N; }

private:
    std::array<char, N> buf{};
    std::size_t len = 0;
    bool overflow = false;
    bool ready = false;
    std::size_t dropped_too_long = 0;
};
