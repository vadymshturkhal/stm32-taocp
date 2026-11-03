#pragma once
#include <cstdint>
#include <cassert>

class KnuthPrimes {
public:
    KnuthPrimes () noexcept{ 
        primes[0] = 2;
        primes[1] = 3; 
    }

    void next_prime() noexcept;
    [[nodiscard]] bool is_done() const noexcept { return primes_found == primes_to_find; }
    [[nodiscard]] std::uint32_t found() const noexcept { return primes_found; }
    [[nodiscard]] std::uint32_t get_prime(std::uint32_t i) const noexcept { 
        assert(i < primes_found);
        return primes[i]; 
    }

private:
    static constexpr std::uint32_t primes_to_find = 500;
    std::uint16_t primes[primes_to_find];
    std::uint32_t primes_found = 2;
    std::uint32_t candidate = 3;  // last found prime

    bool is_prime(std::uint32_t n) noexcept;
};
