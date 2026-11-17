#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include "oled_font.hpp"


/*
 * NumberView is the screen's content, and it's the only part that knows what to draw.
 * OledNode only knows how to get bytes to the screen.
 */

// One number on the screen. The first view: it proves the OLED path before primes use it.
class NumberView {
public:
    static constexpr std::uint8_t TEXT_PAGE = 3;
    static constexpr std::size_t TEXT_X = 60;

    explicit constexpr NumberView(std::uint32_t number) noexcept : value(number) {}

    void snapshot() noexcept {}
    void frame_done(std::uint32_t, bool) noexcept {}

    void render(std::uint8_t page, std::uint8_t* out) const noexcept {
        std::fill_n(out, OLED_WIDTH, std::uint8_t{0});
        if (page != TEXT_PAGE) return;

        // The digits, last one first
        std::array<std::uint8_t, 10> digits{};
        std::size_t count = 0;
        std::uint32_t rest = value;
        do {
            digits[count++] = static_cast<std::uint8_t>(rest % 10);
            rest /= 10;
        } while (rest != 0);

        std::size_t x = TEXT_X;
        while (count != 0 && x + FONT_OLED_WIDTH <= OLED_WIDTH) {
            const Glyph& glyph = DIGITS[digits[--count]];
            std::copy(glyph.begin(), glyph.end(), out + x);
            x += FONT_OLED_WIDTH;
        }
    }

private:
    static constexpr std::size_t OLED_WIDTH = 128;

    std::uint32_t value;
};
