#pragma once
#include <cstdint>
#include "elevator_constants.hpp"

// One user's random values (Knuth's VALUES subroutine)
struct UserValues {
    std::uint32_t IN;
    std::uint32_t OUT;
    std::uint32_t GIVEUPTIME;
    std::uint32_t INTERTIME;    // time until the next user arrives
};

// Knuth's VALUES: xorshift32, the same sequence as the C version.
// Not <random>: std::uniform_int_distribution's algorithm is not specified by the
// standard, so other libraries would give other numbers, and the golden traces
// depend on these exact ones (and on the order of the calls in next()).
class Values {
public:
    explicit constexpr Values(std::uint32_t seed) noexcept : state(seed != 0 ? seed : 1) {}

    constexpr UserValues next() noexcept {
        UserValues v{};
        v.IN = range(0, FLOORS - 1);
        do {
            v.OUT = range(0, FLOORS - 1);
        } while (v.OUT == v.IN);
        v.GIVEUPTIME = range(100, 400);
        v.INTERTIME = range(100, 200);
        return v;
    }

private:
    constexpr std::uint32_t xorshift32() noexcept {
        std::uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state = x;
        return x;
    }

    // Inclusive range [lo, hi]
    constexpr std::uint32_t range(std::uint32_t lo, std::uint32_t hi) noexcept {
        return lo + xorshift32() % (hi - lo + 1);
    }

    std::uint32_t state;    // xorshift32 needs a nonzero state
};

// The first users of seed 1, as the C version gives them (and trace_4.txt shows:
// arrivals at 0, 135, 265; give up at 370, 250, 378). Checked while compiling.
static_assert([] {
    Values v{1};
    const UserValues a = v.next();
    const UserValues b = v.next();
    const UserValues c = v.next();
    return a.IN == 4 && a.OUT == 1 && a.GIVEUPTIME == 370 && a.INTERTIME == 135
        && b.IN == 4 && b.OUT == 2 && b.GIVEUPTIME == 115 && b.INTERTIME == 130
        && c.IN == 2 && c.OUT == 4 && c.GIVEUPTIME == 113 && c.INTERTIME == 170;
}(), "Values must give the same sequence as the C version");
