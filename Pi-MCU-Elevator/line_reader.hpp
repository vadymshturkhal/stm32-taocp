#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

template <std::size_t N>
class LineReader {
    static_assert(N >= 1, "LineCreator needs at least one character for length")

public:
    bool feed(std::uint8_t byte) noexcept {

    }

};