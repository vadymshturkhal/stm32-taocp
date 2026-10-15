// test_wait_list.cpp - tests of WaitList / WaitNode (wait_list.hpp), run on the PC
//
//   is_less:     order and the 32-bit clock wrapping round (static_assert)
//   the list:    empty, order by NEXTTIME, FIFO for equal times, due exactly at
//                NEXTTIME, cancel (twice), re-hold = restart, holdc changes the
//                handler, immed jumps the queue, one node per step
//   handlers:    a periodic node that re-holds itself, a handler that cancels
//                another due node
//   wrap:        deadlines on both sides of 0xFFFFFFFF -> 0
//   random:      100 000 random hold / cancel / time steps across the wrap,
//                compared with a simple model that uses 64-bit time
//
// The owner here plays the Session: it owns the list and the nodes, and step()
// is Session::on_tick().
//
// Build (from tests/):  g++-14 -std=c++23 -I.. -O2 -Wall -Wextra -Wconversion
//             test_wait_list.cpp -o test_wait_list
// Exit status: 0 if every check passed.
#include <array>
#include <cstdint>
#include <print>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

#include "wait_list.hpp"

static int failures = 0;

static void check(const char* name, bool ok) {
    std::println("{}  {}", ok ? "PASS" : "FAIL", name);
    failures += !ok;
}

// is_less is constexpr, so its rules are checked while compiling.
template <class C> using WL = WaitList<C>;
struct Any {};
static_assert( WL<Any>::is_less(1, 2));
static_assert(!WL<Any>::is_less(2, 1));
static_assert(!WL<Any>::is_less(5, 5));
static_assert( WL<Any>::is_less(0xFFFFFFF0u, 0x10u));    // 0x10 comes after the wrap
static_assert(!WL<Any>::is_less(0x10u, 0xFFFFFFF0u));

// Nodes hold pointers to their neighbours, so they must not be copied or moved.
static_assert(!std::is_copy_constructible_v<WaitNode<Any>>);
static_assert(!std::is_copy_assignable_v<WaitNode<Any>>);
static_assert(!std::is_move_constructible_v<WaitNode<Any>>);
static_assert(!std::is_copy_constructible_v<WaitList<Any>>);

// The owner: handlers append a letter to log, so the order they ran in is a string.
struct Owner {
    using List = WaitList<Owner>;
    List wait;
    List::Node a, b, c, period;
    std::string log;

    void on_a(std::uint32_t) noexcept { log += 'a'; }
    void on_b(std::uint32_t) noexcept { log += 'b'; }
    void on_c(std::uint32_t) noexcept { log += 'c'; }
    void on_period(std::uint32_t now) noexcept { log += 'p'; wait.hold(period, now, 10); }  // re-holds itself
    void cancel_b(std::uint32_t) noexcept { log += 'x'; wait.delete_node(b); }

    // Session::on_tick(): run at most one due node. Returns whether one ran.
    bool step(std::uint32_t now) noexcept {
        if (!wait.is_node_ready(now)) return false;
        if (const auto fn = wait.delete_first()) (this->*fn)(now);
        return true;
    }

    // Run every node due at now, one step at a time.
    void run_until(std::uint32_t now) noexcept { while (step(now)) {} }
};

// ---- The random test: 8 nodes, each with its own handler that records its number ----

struct Random {
    using List = WaitList<Random>;
    static constexpr std::size_t N = 8;
    List wait;
    std::array<List::Node, N> nodes;
    std::vector<int> fired;

    template <int I> void fire(std::uint32_t) noexcept { fired.push_back(I); }
    static constexpr std::array<List::NextInst, N> HANDLERS{
        &Random::fire<0>, &Random::fire<1>, &Random::fire<2>, &Random::fire<3>,
        &Random::fire<4>, &Random::fire<5>, &Random::fire<6>, &Random::fire<7>};
};

// The model: no list at all, just "is it armed, when, and in which order was it held".
// 64-bit time never wraps, so it shows what the 32-bit list must do.
struct Model {
    struct Entry { bool armed = false; std::uint64_t when = 0; std::uint64_t seq = 0; };
    std::array<Entry, Random::N> e{};
    std::uint64_t next_seq = 0;

    void hold(std::size_t i, std::uint64_t when) { e[i] = {true, when, next_seq++}; }
    void cancel(std::size_t i) { e[i].armed = false; }

    // The node that should run at now: the earliest due one, the first held among equals.
    int due(std::uint64_t now) const {
        int best = -1;
        for (std::size_t i = 0; i < e.size(); ++i) {
            if (!e[i].armed || e[i].when > now) continue;
            if (best < 0 || e[i].when < e[best].when ||
                (e[i].when == e[best].when && e[i].seq < e[best].seq)) best = static_cast<int>(i);
        }
        return best;
    }
};

int main() {
    // 1. A new list
    {
        Owner o;
        check("new list: empty, nothing ready, delete_first gives nullptr",
              o.wait.is_empty() && !o.wait.is_node_ready(0) && o.wait.delete_first() == nullptr);
        o.wait.delete_node(o.a);             // a node that was never held
        check("delete_node on a node that was never held: harmless", o.wait.is_empty());
    }

    // 2. Order by NEXTTIME
    {
        Owner o;
        o.wait.holdc(o.a, 0, 300, &Owner::on_a);
        o.wait.holdc(o.b, 0, 100, &Owner::on_b);
        o.wait.holdc(o.c, 0, 200, &Owner::on_c);
        o.run_until(1000);
        check("held 300a, 100b, 200c: run b c a, then empty", o.log == "bca" && o.wait.is_empty());
    }

    // 3. Equal times keep the order they were held in
    {
        Owner o;
        o.wait.holdc(o.a, 0, 50, &Owner::on_a);
        o.wait.holdc(o.b, 0, 50, &Owner::on_b);
        o.wait.holdc(o.c, 0, 50, &Owner::on_c);
        o.run_until(50);
        check("three nodes at the same time: FIFO (a b c)", o.log == "abc");
    }

    // 4. Due exactly at NEXTTIME, and only once
    {
        Owner o;
        o.wait.holdc(o.a, 1000, 4000, &Owner::on_a);
        const bool early = o.step(4999);
        const bool on_time = o.step(5000);
        const bool again = o.step(5000);
        check("held at 1000 for 4000: not at 4999, runs at 5000, then not again",
              !early && on_time && !again && o.log == "a");
    }

    // 5. Late is fine: a node that is overdue runs at the next step
    {
        Owner o;
        o.wait.holdc(o.a, 0, 10, &Owner::on_a);
        o.run_until(100000);
        check("overdue node runs at the next step", o.log == "a");
    }

    // 6. Cancel, also twice
    {
        Owner o;
        o.wait.holdc(o.a, 0, 100, &Owner::on_a);
        o.wait.holdc(o.b, 0, 200, &Owner::on_b);
        o.wait.delete_node(o.a);
        o.wait.delete_node(o.a);
        o.run_until(1000);
        check("cancel a (twice): only b runs", o.log == "b" && o.wait.is_empty());
    }

    // 7. Cancel, then hold again
    {
        Owner o;
        o.wait.holdc(o.a, 0, 100, &Owner::on_a);
        o.wait.delete_node(o.a);
        o.wait.hold(o.a, 0, 300);
        o.run_until(299);
        const std::string before = o.log;
        o.run_until(300);
        check("cancel, then hold again: runs at the new time", before.empty() && o.log == "a");
    }

    // 8. Re-hold a node that is still in the list = restart
    {
        Owner o;
        o.wait.holdc(o.a, 0, 100, &Owner::on_a);
        o.wait.holdc(o.b, 0, 200, &Owner::on_b);
        o.wait.hold(o.a, 0, 300);            // a moves from first to last
        o.run_until(1000);
        check("re-hold a from 100 to 300: b a, a runs once", o.log == "ba");
    }
    {
        Owner o;
        o.wait.holdc(o.a, 0, 100, &Owner::on_a);
        o.wait.holdc(o.b, 0, 200, &Owner::on_b);
        o.wait.holdc(o.c, 0, 300, &Owner::on_c);
        o.wait.hold(o.c, 0, 50);             // c moves from last to first
        o.wait.hold(o.b, 0, 200);            // b held again at the same time: stays where it was
        o.run_until(1000);
        check("re-hold c from 300 to 50, b at the same time: c a b", o.log == "cab");
    }
    {
        Owner o;                              // like peer_silent_: restarted on every line
        o.wait.holdc(o.a, 0, 100, &Owner::on_a);
        for (std::uint32_t now = 10; now <= 500; now += 10) {
            o.wait.hold(o.a, now, 100);
            o.run_until(now);
        }
        const std::string kept_alive = o.log;
        o.run_until(600);
        check("restarted every 10 ms: never runs; runs 100 ms after the last restart",
              kept_alive.empty() && o.log == "a");
    }

    // 9. holdc replaces the handler
    {
        Owner o;
        o.wait.holdc(o.a, 0, 10, &Owner::on_a);
        o.wait.holdc(o.a, 0, 10, &Owner::on_b);
        o.run_until(10);
        check("holdc twice on one node: the second handler runs, once", o.log == "b");
    }

    // 10. immed jumps the queue, even ahead of overdue nodes
    {
        Owner o;
        o.wait.holdc(o.a, 0, 10, &Owner::on_a);
        o.wait.holdc(o.b, 0, 20, &Owner::on_b);
        o.wait.holdc(o.c, 0, 999, &Owner::on_c);
        o.wait.immed(o.c, 100);              // a and b are overdue at 100
        o.run_until(100);
        check("immed c at 100 with a, b overdue: c a b", o.log == "cab" && o.wait.is_empty());
    }

    // 11. One node per step: the main loop gets a turn between them
    {
        Owner o;
        o.wait.holdc(o.a, 0, 10, &Owner::on_a);
        o.wait.holdc(o.b, 0, 10, &Owner::on_b);
        o.wait.holdc(o.c, 0, 10, &Owner::on_c);
        o.step(10);
        const std::string first = o.log;
        o.step(10);
        check("three due nodes: one step runs one", first == "a" && o.log == "ab");
    }

    // 12. A periodic node that re-holds itself
    {
        Owner o;
        o.wait.holdc(o.period, 0, 10, &Owner::on_period);
        for (std::uint32_t now = 0; now <= 100; ++now) o.run_until(now);
        const std::string ticks = o.log;
        o.run_until(1000);                   // a long stall
        check("period 10 over 100 ms: 10 runs; after a stall: 1 run, no endless loop",
              ticks == std::string(10, 'p') && o.log == std::string(11, 'p') && !o.wait.is_empty());
    }

    // 13. A handler that cancels another node that is also due
    {
        Owner o;
        o.wait.holdc(o.a, 0, 10, &Owner::cancel_b);
        o.wait.holdc(o.b, 0, 10, &Owner::on_b);
        o.run_until(10);
        check("handler cancels a due node: it never runs", o.log == "x" && o.wait.is_empty());
    }

    // 14. The clock wrapping round
    {
        Owner o;
        o.wait.holdc(o.a, 0xFFFFFF00u, 0x200, &Owner::on_a);   // due at 0x100, after the wrap
        o.wait.holdc(o.b, 0xFFFFFF00u, 0x80, &Owner::on_b);    // due at 0xFFFFFF80, before it
        const bool b_early = o.wait.is_node_ready(0xFFFFFF7Fu);
        o.run_until(0xFFFFFFFFu);
        const std::string before_wrap = o.log;
        const bool a_early = o.wait.is_node_ready(0xFFFFFFFFu) || o.wait.is_node_ready(0xFFu);
        o.run_until(0x100);
        check("wrap: b due before 0xFFFFFFFF runs then, a due at 0x100 runs after the wrap",
              !b_early && before_wrap == "b" && !a_early && o.log == "ba");
    }
    {
        Owner o;                              // the SYN_RCVD timeout held just before the wrap
        o.wait.holdc(o.a, 0xFFFFF000u, 4096, &Owner::on_a);    // due at exactly 0
        check("4096 ms from 0xFFFFF000: not ready at 0xFFFFFFFF, ready at 0",
              !o.wait.is_node_ready(0xFFFFFFFFu) && o.wait.is_node_ready(0));
    }

    // 15. Random holds, cancels and time steps, against the model
    {
        Random r;
        Model m;
        std::mt19937 rng(12345);
        std::uniform_int_distribution<std::size_t> pick(0, Random::N - 1);
        std::uniform_int_distribution<int> op(0, 9);
        std::uniform_int_distribution<std::uint32_t> delay(0, 300);
        std::uniform_int_distribution<std::uint32_t> advance(0, 40);

        std::uint64_t now64 = 0xFFFF0000u;   // starts 65536 ms before the 32-bit wrap
        bool same = true;
        std::size_t runs = 0;
        for (int i = 0; i < 100000 && same; ++i) {
            const auto now = static_cast<std::uint32_t>(now64);
            const int o = op(rng);
            const std::size_t n = pick(rng);
            if (o < 5) {                     // hold (sometimes a restart)
                const std::uint32_t d = delay(rng);
                r.wait.holdc(r.nodes[n], now, d, Random::HANDLERS[n]);
                m.hold(n, now64 + d);
            } else if (o < 7) {              // cancel
                r.wait.delete_node(r.nodes[n]);
                m.cancel(n);
            } else {                         // time passes; run what is due, one per step
                now64 += advance(rng);
                const auto t = static_cast<std::uint32_t>(now64);
                for (;;) {
                    const int expect = m.due(now64);
                    const bool ready = r.wait.is_node_ready(t);
                    if (ready != (expect >= 0)) { same = false; break; }
                    if (!ready) break;
                    r.fired.clear();
                    if (const auto fn = r.wait.delete_first()) (r.*fn)(t);
                    if (r.fired.size() != 1 || r.fired[0] != expect) { same = false; break; }
                    m.cancel(static_cast<std::size_t>(expect));
                    ++runs;
                }
            }
        }
        std::println("      ({} handler runs, time reached {:#x})", runs, now64);
        check("100 000 random hold / cancel / time steps across the wrap: same as the model",
              same && now64 > 0x100000000u);
    }

    std::println("\n{}", failures ? "SOME TESTS FAILED" : "all passed");
    return failures != 0;
}
