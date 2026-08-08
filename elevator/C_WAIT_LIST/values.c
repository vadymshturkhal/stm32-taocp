#include "values.h"
#include "elevator_settings.h"

// xorshift32 requires a nonzero state
static uint32_t rng_state = 1;

void values_seed(uint32_t seed) {
    rng_state = (seed != 0) ? seed : 1;
}

static uint32_t xorshift32(void) {
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x;
    return x;
}

// Inclusive range [lo, hi]
static uint32_t rand_range(uint32_t lo, uint32_t hi) {
    return lo + (xorshift32() % (hi - lo + 1));
}

Values values(void) {
    Values result;

    result.IN = rand_range(0, FLOORS - 1);

    uint32_t out;
    do {
        out = rand_range(0, FLOORS - 1);
    } while (out == result.IN);
    result.OUT = out;

    result.GIVEUPTIME = rand_range(100, 400);
    result.INTERTIME = rand_range(100, 200);

    return result;
}
