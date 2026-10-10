#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

template <std::size_t N>
// Copy each byte to the internal buffer
class LineReader {
    static_assert(N >= 1, "LineReader needs at least one character for length");

public:
    bool feed(std::uint8_t byte) noexcept {

    }

    std::string_view line() const noexcept { return {buf.data(), len}; }

    std::size_t dropped() const noexcept { return dropped_too_long; }

    static constexpr std::size_t size() noexcept { return N; }

private:
    std::array<char, N> buf{};
    std::size_t len = 0;
    bool overflow = false;
    bool ready = false;
    std::size_t dropped_too_long = 0;
};
