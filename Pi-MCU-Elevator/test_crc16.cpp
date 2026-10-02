#include <array>
#include <utility>
#include <print>
#include <format>

#include "crc16.hpp"

static_assert(crc16("") == 0xFFFF, "empty input");
static_assert(crc16("A") == 0xB915, "A");
static_assert(crc16("123456789") == 0x29B1, "123456789");
static_assert(crc16("0") == 0xD7A3, "0");
static_assert(crc16("2 0 5 hello") == 0xEBDC, "2 0 5 hello");

constexpr std::array<std::pair<std::string_view, std::uint16_t>, 5> TEST_CASES {{
    {"", 0xFFFF},
    {"A", 0xB915},
    {"123456789", 0x29B1},
    {"0", 0xD7A3},
    {"2 0 5 hello", 0xEBDC},
}};

int main() {
    std::size_t passed = 0;

    for (auto [text, expected] : TEST_CASES) {
        std::span<const std::uint8_t> bytes {
            reinterpret_cast<const std::uint8_t*>(text.data()), 
            text.size()
        };

        std::uint16_t got = crc16(bytes);

        bool ok = got == expected;
        if (ok) ++passed;

        const char* result = ok ? "PASS" : "FAIL";
        std::println("{}  {:<16}  got {:04X}  want {:04X}",
             result, std::format("\"{}\"", text), got, expected);

    }

    std::println("\n{}/{} passed", passed, TEST_CASES.size());
    return passed != TEST_CASES.size();
}


