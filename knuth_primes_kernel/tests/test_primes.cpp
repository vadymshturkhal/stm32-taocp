// test_primes.cpp - tests of Algorithm P (knuth_primes.hpp) and its kernel node
// (primes_node.hpp), run on the PC
//
//   the table:  starts with 2 and 3; each next_prime adds one prime; all 500 match
//               a plain trial division; a call on a full table changes nothing
//   the node:   one prime per step; it holds itself by its own delay, counted from
//               now; it leaves the list when the table is full
//   time:       a run started late and a run across the 32-bit clock wrap; another
//               program beside it runs at the same times as it does alone
//
// Build (from tests/):  g++-14 -std=c++23 -I.. -I../../knuth_kernel -O2 -Wall -Wextra -Wshadow
//             -Wconversion -Wsign-conversion test_primes.cpp ../knuth_primes.cpp -o test_primes
// Exit status: 0 if every check passed.
#include <cstdint>
#include <print>
#include <vector>

#include "primes_node.hpp"

static int failures = 0;

static void check(const char* name, bool ok) {
    std::println("{}  {}", ok ? "PASS" : "FAIL", name);
    failures += !ok;
}

// Knuth's table in §1.3.2 ends at 3571
static constexpr std::uint32_t PRIMES = 500;
static constexpr std::uint32_t LAST_PRIME = 3571;
static constexpr std::uint32_t STEPS = PRIMES - 2;   // the table starts with 2 and 3

// The reference: trial division by every number, nothing shared with Algorithm P
static std::vector<std::uint32_t> first_primes(std::uint32_t how_many) {
    std::vector<std::uint32_t> result;
    for (std::uint32_t n = 2; result.size() < how_many; ++n) {
        bool prime = true;
        for (std::uint32_t d = 2; d * d <= n; ++d) {
            if (n % d == 0) { prime = false; break; }
        }
        if (prime) result.push_back(n);
    }
    return result;
}

static bool matches_reference(const KnuthPrimes& table) {
    const std::vector<std::uint32_t> reference = first_primes(PRIMES);
    for (std::uint32_t i = 0; i < PRIMES; ++i) {
        if (table.get_prime(i) != reference[i]) return false;
    }
    return true;
}

// Event-driven, like the elevator's CYCLE: jump to each deadline until the list is empty
static std::uint32_t run_to_end(Kernel& k) {
    std::uint32_t steps = 0;
    while (const auto due = k.get_next_time()) {
        k.step(*due);
        ++steps;
    }
    return steps;
}

// Another program: runs a fixed number of times and adds up the times it ran at
struct Ticker : WaitNode {
    Ticker(Kernel& k, std::uint32_t delay, std::uint32_t runs_wanted) noexcept
        : WaitNode(tick, delay), kernel(&k), runs(runs_wanted) {}

    static void tick(WaitNode& node, std::uint32_t now) noexcept {
        auto& t = static_cast<Ticker&>(node);
        t.time_sum += now;
        if (++t.count < t.runs) t.kernel->hold(node, now);
    }

    Kernel* kernel;
    std::uint32_t runs;
    std::uint32_t count = 0;
    std::uint32_t time_sum = 0;
};

int main() {
    // 1. A new table
    {
        KnuthPrimes table;
        check("new table: 2 and 3, not full",
              table.found() == 2 && table.get_prime(0) == 2 && table.get_prime(1) == 3 && !table.is_done());
    }

    // 2. Each call adds exactly one prime, and passes over a composite
    {
        KnuthPrimes table;
        table.next_prime();
        check("next_prime: 5 is the third prime, stored once", table.found() == 3 && table.get_prime(2) == 5);
        table.next_prime();
        table.next_prime();
        check("next_prime: 9 is passed over, 7 then 11", table.found() == 5 && table.get_prime(3) == 7 && table.get_prime(4) == 11);
    }

    // 3. The whole table
    {
        KnuthPrimes table;
        std::uint32_t calls = 0;
        while (!table.is_done()) {
            table.next_prime();
            ++calls;
        }
        check("full table: 498 calls", calls == STEPS);
        check("full table: 500 primes, the last is 3571", table.found() == PRIMES && table.get_prime(PRIMES - 1) == LAST_PRIME);
        check("full table: all 500 match trial division", matches_reference(table));

        // P3: without it this writes past the array (the sanitizer build sees it)
        table.next_prime();
        check("a call on a full table changes nothing", table.found() == PRIMES && table.get_prime(PRIMES - 1) == LAST_PRIME);
    }

    // 4. The node: one prime per step, and out of the list at the end
    {
        Kernel k;
        PrimesNode node(k, 10);
        k.start(node, 0);
        k.step(0);
        check("node: one step adds one prime", node.knuth_primes.found() == 3);
        check("node: it holds itself", node.is_held() && k.get_next_time() == 10u);

        const std::uint32_t steps = 1 + run_to_end(k);
        check("node: 498 steps", steps == STEPS);
        check("node: all 500 primes match trial division", matches_reference(node.knuth_primes));
        check("node: out of the list when full", !node.is_held() && !k.get_next_time());
        check("node: nothing refused", k.refused() == 0);
    }

    // 5. The delay counts from now, not from 0. Started late, as on a board that
    //    has been up for a while: hold(node, delay) would pass this as the time
    {
        Kernel k;
        PrimesNode node(k, 1000);
        const std::uint32_t start = 1'000'000;
        k.start(node, start);
        k.step(start);
        check("late start: next due is now + delay", k.get_next_time() == start + 1000);
        const std::uint32_t steps = 1 + run_to_end(k);
        check("late start: 498 steps", steps == STEPS);
    }

    // 6. Across the wrap of the 32-bit clock, in the middle of the table
    {
        Kernel k;
        PrimesNode node(k, 1000);
        const std::uint32_t start = 0u - 250'000u;   // 250 steps before the wrap
        k.start(node, start);

        std::uint32_t steps = 0;
        std::uint32_t last = start;
        while (const auto due = k.get_next_time()) {
            last = *due;
            k.step(last);
            ++steps;
        }
        check("wrap: 498 steps, all primes match", steps == STEPS && matches_reference(node.knuth_primes));
        check("wrap: the last step is on time", last == start + (STEPS - 1) * 1000);
        check("wrap: out of the list when full", !node.is_held() && !k.get_next_time());
    }

    // 7. Beside another program: the other one runs at the times it runs alone
    {
        const auto run_ticker = [](bool with_primes) {
            Kernel k;
            Ticker ticker(k, 7, 1000);
            PrimesNode node(k, 3);
            k.start(ticker, 0);
            if (with_primes) k.start(node, 0);
            run_to_end(k);
            return ticker.time_sum;
        };
        check("beside another program: its times do not change", run_ticker(true) == run_ticker(false));
    }

    std::println("\n{}", failures ? "SOME TESTS FAILED" : "all passed");
    return failures != 0;
}
