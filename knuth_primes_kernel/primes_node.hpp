#pragma once
#include <cstdint>
#include "kernel.hpp"
#include "knuth_primes.hpp"

class PrimesNode : public WaitNode {
public:
    PrimesNode(Kernel& k, std::uint32_t delay) noexcept : WaitNode(step, delay), kernel(k) {}

    KnuthPrimes knuth_primes;
private:
    Kernel& kernel;

    static void step(WaitNode& node, std::uint32_t now) noexcept {
        auto& prime_node = static_cast<PrimesNode&>(node);
        prime_node.knuth_primes.next_prime();
        if (!prime_node.knuth_primes.is_full()) { prime_node.kernel.hold(prime_node, now); }
        // Else: already canceled
    }
};
