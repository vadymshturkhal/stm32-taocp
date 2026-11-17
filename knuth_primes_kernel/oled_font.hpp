#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

// The 6x8 digits of the ssd1306 library (afiskon/stm32-ssd1306, MIT). One value per
// row, top row first, the leftmost pixel in bit 15.
inline constexpr std::size_t FONT_WIDTH = 6;
inline constexpr std::size_t FONT_HEIGHT = 8;

inline constexpr std::uint16_t DIGIT_ROWS[10][FONT_HEIGHT] = {
    {0x7000, 0x8800, 0x9800, 0xa800, 0xc800, 0x8800, 0x7000, 0x0000},   // 0
    {0x2000, 0x6000, 0x2000, 0x2000, 0x2000, 0x2000, 0x7000, 0x0000},   // 1
    {0x7000, 0x8800, 0x0800, 0x7000, 0x8000, 0x8000, 0xf800, 0x0000},   // 2
    {0xf800, 0x0800, 0x1000, 0x3000, 0x0800, 0x8800, 0x7000, 0x0000},   // 3
    {0x1000, 0x3000, 0x5000, 0x9000, 0xf800, 0x1000, 0x1000, 0x0000},   // 4
    {0xf800, 0x8000, 0xf000, 0x0800, 0x0800, 0x8800, 0x7000, 0x0000},   // 5
    {0x3800, 0x4000, 0x8000, 0xf000, 0x8800, 0x8800, 0x7000, 0x0000},   // 6
    {0xf800, 0x0800, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000, 0x0000},   // 7
    {0x7000, 0x8800, 0x8800, 0x7000, 0x8800, 0x8800, 0x7000, 0x0000},   // 8
    {0x7000, 0x8800, 0x8800, 0x7800, 0x0800, 0x1000, 0xe000, 0x0000},   // 9
};

using Glyph = std::array<std::uint8_t, FONT_WIDTH>;

// The SSD1306 takes columns: one byte is 8 pixels high, the top pixel in bit 0.
// Row to column at compile time, so a character is a 6-byte copy in a step.
constexpr std::array<Glyph, 10> make_digit_columns() noexcept {
    std::array<Glyph, 10> digits{};
    for (std::size_t digit = 0; digit < digits.size(); ++digit) {
        for (std::size_t column = 0; column < FONT_WIDTH; ++column) {
            for (std::size_t row = 0; row < FONT_HEIGHT; ++row) {
                if (DIGIT_ROWS[digit][row] & (0x8000u >> column)) {
                    digits[digit][column] |= static_cast<std::uint8_t>(1u << row);
                }
            }
        }
    }
    return digits;
}

inline constexpr std::array<Glyph, 10> DIGITS = make_digit_columns();
