#pragma once
#include <cstdint>


class KnuthPrimes {
public:
    KnuthPrimes () { 
        primes[0] = 2;
        primes[1] = 3; 
    }

    void next_prime() noexcept;
    [[nodiscard]] bool is_full() const noexcept { return primes_found == primes_to_find; }
    [[nodiscard]] std::uint32_t found() const noexcept { return primes_found; }
    [[nodiscard]] std::uint32_t prime(std::uint32_t i) const noexcept { return primes[i]; }

private:
    static constexpr std::uint32_t primes_to_find = 500;
    std::uint32_t primes[primes_to_find];
    std::uint32_t primes_found = 2;
    std::uint32_t candidate = 3;

    bool is_prime(std::uint32_t candidate) noexcept;
};
