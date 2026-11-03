#include "knuth_primes.hpp"

bool KnuthPrimes::is_prime(std::uint32_t n) noexcept {
    std::uint32_t Q;
    std::uint32_t R;

    // P5
    std::uint32_t primes_counter = 0;  // start from 3 as a divisor

    // P7
    do {
        primes_counter += 1;

        // P6
        Q = n / primes[primes_counter];
        R = n % primes[primes_counter];

        if (R == 0) {
            // to P4
            return false;
        }
    } while (Q > primes[primes_counter]);

    return true;
}

void KnuthPrimes::next_prime() noexcept {
    // [P3]
    if (is_done()) { return; }

    do {
        // [P4]
        candidate += 2;
    } while (!is_prime(candidate));

    // [P2]
    primes[primes_found] = static_cast<std::uint16_t>(candidate);
    ++primes_found;
}
