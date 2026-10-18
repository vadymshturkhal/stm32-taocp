// test_wait_list.cpp - tests of the kernel's WaitList / WaitNode (wait_list.hpp), run on the PC
//
//   is_less:     order and the 32-bit clock wrapping round (static_assert)
//   the list:    empty, order by NEXTTIME, FIFO for equal times, due exactly at
//                NEXTTIME, cancel (twice), re-hold = restart, the node's own DELAY,
//                holdc changes the handler, next_inst / set_next_inst (also on a node
//                that is waiting), immed jumps the queue, one node per step
//   params:      one handler shared by many nodes, each node carrying its own data
//                (a node type derived from WaitNode, like Knuth's user nodes)
//   handlers:    a periodic node that re-holds itself, a handler that cancels
//                another due node
//   wrap:        deadlines on both sides of 0xFFFFFFFF -> 0
//   random:      100 000 random hold / cancel / time steps across the wrap,
//                compared with a simple model that uses 64-bit time
//
// step() below is one step of the kernel: run the first node, if it is due.
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
static_assert( WaitList::is_less(1, 2));
static_assert(!WaitList::is_less(2, 1));
static_assert(!WaitList::is_less(5, 5));
static_assert( WaitList::is_less(0xFFFFFFF0u, 0x10u));    // 0x10 comes after the wrap
static_assert(!WaitList::is_less(0x10u, 0xFFFFFFF0u));

// Nodes hold pointers to their neighbours, so they must not be copied or moved.
static_assert(!std::is_copy_constructible_v<WaitNode>);
static_assert(!std::is_copy_assignable_v<WaitNode>);
static_assert(!std::is_move_constructible_v<WaitNode>);
static_assert(!std::is_copy_constructible_v<WaitList>);

// One step of the kernel. Returns whether a node ran.
static bool step(WaitList& wait, std::uint32_t now) noexcept {
    if (!wait.is_node_ready(now)) return false;
    if (WaitNode* n = wait.delete_first()) n->run(now);
    return true;
}

// Run every node due at now, one step at a time.
static void run_until(WaitList& wait, std::uint32_t now) noexcept {
    while (step(wait, now)) {}
}

// ---- A small "program" for the tests: nodes that write a letter into a log ----

struct World;

// A program's node: WaitNode plus its own params, like Knuth's user nodes (IN, OUT).
struct LetterNode : WaitNode {
    LetterNode(World& w, char c, std::uint32_t delay = 0) noexcept;
    World* world;
    char letter;
};

struct World {
    WaitList wait;
    std::string log;
    LetterNode a{*this, 'a'}, b{*this, 'b'}, c{*this, 'c'};
    LetterNode period{*this, 'p', 10};                     // its own DELAY: every 10 ms
};

// One handler for every LetterNode: the node tells it which letter and which log.
static void write_letter(WaitNode& n, std::uint32_t) noexcept {
    auto& node = static_cast<LetterNode&>(n);
    node.world->log += node.letter;
}

// Another handler, to check that holdc replaces it: the letter in upper case.
static void write_upper(WaitNode& n, std::uint32_t) noexcept {
    auto& node = static_cast<LetterNode&>(n);
    node.world->log += static_cast<char>(node.letter - 'a' + 'A');
}

// A periodic handler: writes its letter, then holds itself again with its own DELAY.
static void write_and_repeat(WaitNode& n, std::uint32_t now) noexcept {
    auto& node = static_cast<LetterNode&>(n);
    node.world->log += node.letter;
    node.world->wait.hold(node, now);
}

// A handler that cancels another node: writes 'x' and removes b.
static void cancel_b(WaitNode& n, std::uint32_t) noexcept {
    auto& node = static_cast<LetterNode&>(n);
    node.world->log += 'x';
    node.world->wait.delete_node(node.world->b);
}

LetterNode::LetterNode(World& w, char c, std::uint32_t delay) noexcept
    : WaitNode(&write_letter, delay), world(&w), letter(c) {}

// ---- The random test: 8 nodes, one handler, each node records its own number ----

struct NumberNode : WaitNode {
    NumberNode() noexcept : WaitNode(&record, 0) {}
    std::size_t number = 0;
    std::vector<std::size_t>* fired = nullptr;

    static void record(WaitNode& n, std::uint32_t) noexcept {
        auto& node = static_cast<NumberNode&>(n);
        node.fired->push_back(node.number);
    }
};

// The model: no list at all, just "is it armed, when, and in which order was it held".
// 64-bit time never wraps, so it shows what the 32-bit list must do.
struct Model {
    static constexpr std::size_t N = 8;
    struct Entry { bool armed = false; std::uint64_t when = 0; std::uint64_t seq = 0; };
    std::array<Entry, N> e{};
    std::uint64_t next_seq = 0;

    void hold(std::size_t i, std::uint64_t when) { e[i] = {true, when, next_seq++}; }
    void cancel(std::size_t i) { e[i].armed = false; }

    // The node that should run at now: the earliest due one, the first held among equals.
    int due(std::uint64_t now) const {
        int best = -1;
        for (std::size_t i = 0; i < e.size(); ++i) {
            if (!e[i].armed || e[i].when > now) continue;
            const auto b = static_cast<std::size_t>(best);
            if (best < 0 || e[i].when < e[b].when || (e[i].when == e[b].when && e[i].seq < e[b].seq))
                best = static_cast<int>(i);
        }
        return best;
    }
};

int main() {
    // 1. A new list
    {
        World w;
        check("new list: empty, nothing ready, delete_first gives nullptr",
              w.wait.is_empty() && !w.wait.is_node_ready(0) && w.wait.delete_first() == nullptr);
        w.wait.delete_node(w.a);             // a node that was never held
        check("delete_node on a node that was never held: harmless", w.wait.is_empty());
    }

    // 2. Order by NEXTTIME
    {
        World w;
        w.wait.hold(w.a, 0, 300);
        w.wait.hold(w.b, 0, 100);
        w.wait.hold(w.c, 0, 200);
        run_until(w.wait, 1000);
        check("held 300a, 100b, 200c: run b c a, then empty", w.log == "bca" && w.wait.is_empty());
    }

    // 3. Equal times keep the order they were held in
    {
        World w;
        w.wait.hold(w.a, 0, 50);
        w.wait.hold(w.b, 0, 50);
        w.wait.hold(w.c, 0, 50);
        run_until(w.wait, 50);
        check("three nodes at the same time: FIFO (a b c)", w.log == "abc");
    }

    // 4. Due exactly at NEXTTIME, and only once
    {
        World w;
        w.wait.hold(w.a, 1000, 4000);
        const bool early = step(w.wait, 4999);
        const bool on_time = step(w.wait, 5000);
        const bool again = step(w.wait, 5000);
        check("held at 1000 for 4000: not at 4999, runs at 5000, then not again",
              !early && on_time && !again && w.log == "a");
    }

    // 5. Late is fine: a node that is overdue runs at the next step
    {
        World w;
        w.wait.hold(w.a, 0, 10);
        run_until(w.wait, 100000);
        check("overdue node runs at the next step", w.log == "a");
    }

    // 6. One handler, many nodes: each node brings its own params
    {
        World w;
        w.wait.hold(w.c, 0, 1);
        w.wait.hold(w.a, 0, 2);
        w.wait.hold(w.b, 0, 3);
        run_until(w.wait, 10);
        check("one handler on three nodes: each writes its own letter (c a b)", w.log == "cab");
    }

    // 7. The node's own DELAY
    {
        World w;
        w.wait.hold(w.period, 100);          // DELAY 10: due at 110
        const bool early = w.wait.is_node_ready(109);
        run_until(w.wait, 110);
        check("hold(node, now) uses the node's DELAY: not at 109, runs at 110",
              !early && w.log == "p");
    }

    // 8. Cancel, also twice
    {
        World w;
        w.wait.hold(w.a, 0, 100);
        w.wait.hold(w.b, 0, 200);
        w.wait.delete_node(w.a);
        w.wait.delete_node(w.a);
        run_until(w.wait, 1000);
        check("cancel a (twice): only b runs", w.log == "b" && w.wait.is_empty());
    }

    // 9. Cancel, then hold again
    {
        World w;
        w.wait.hold(w.a, 0, 100);
        w.wait.delete_node(w.a);
        w.wait.hold(w.a, 0, 300);
        run_until(w.wait, 299);
        const std::string before = w.log;
        run_until(w.wait, 300);
        check("cancel, then hold again: runs at the new time", before.empty() && w.log == "a");
    }

    // 10. Re-hold a node that is still in the list = restart
    {
        World w;
        w.wait.hold(w.a, 0, 100);
        w.wait.hold(w.b, 0, 200);
        w.wait.hold(w.a, 0, 300);            // a moves from first to last
        run_until(w.wait, 1000);
        check("re-hold a from 100 to 300: b a, a runs once", w.log == "ba");
    }
    {
        World w;
        w.wait.hold(w.a, 0, 100);
        w.wait.hold(w.b, 0, 200);
        w.wait.hold(w.c, 0, 300);
        w.wait.hold(w.c, 0, 50);             // c moves from last to first
        w.wait.hold(w.b, 0, 200);            // b held again at the same time: stays where it was
        run_until(w.wait, 1000);
        check("re-hold c from 300 to 50, b at the same time: c a b", w.log == "cab");
    }
    {
        World w;                              // a watchdog: restarted every 10 ms
        for (std::uint32_t now = 10; now <= 500; now += 10) {
            w.wait.hold(w.a, now, 100);
            run_until(w.wait, now);
        }
        const std::string kept_alive = w.log;
        run_until(w.wait, 600);
        check("restarted every 10 ms: never runs; runs 100 ms after the last restart",
              kept_alive.empty() && w.log == "a");
    }

    // 11. holdc replaces the handler
    {
        World w;
        w.wait.hold(w.a, 0, 10);
        w.wait.holdc(w.a, 0, 10, &write_upper);
        run_until(w.wait, 10);
        check("holdc on a held node: the new handler runs, once", w.log == "A");
    }
    {
        World w;
        w.wait.holdc(w.period, 0, &write_upper);   // holdc with the node's DELAY (10)
        run_until(w.wait, 9);
        const std::string early = w.log;
        run_until(w.wait, 10);
        check("holdc(node, now, fn) uses the node's DELAY", early.empty() && w.log == "P");
    }

    // 11b. next_inst / set_next_inst: the node's next step is data that can be read and changed
    {
        World w;
        const bool born_with = w.a.next_inst() == &write_letter;
        w.wait.holdc(w.a, 0, 10, &write_upper);
        const bool after_holdc = w.a.next_inst() == &write_upper;
        check("next_inst: the step the node was created with, then the one holdc set",
              born_with && after_holdc);
    }
    {
        World w;
        w.wait.hold(w.a, 0, 50);             // a is held for 50 with write_letter
        w.wait.hold(w.b, 0, 30);
        w.a.set_next_inst(&write_upper);     // change a's next step while it waits
        const bool still_held = w.a.is_held();
        run_until(w.wait, 49);
        const std::string before = w.log;
        run_until(w.wait, 50);
        check("set_next_inst on a held node: still held, same time (50), the new step runs",
              still_held && before == "b" && w.log == "bA");
    }
    {
        World w;                              // Knuth's CYCLE1 / the elevator's E1A
        w.a.set_next_inst(&write_upper);      // set the next step without scheduling it
        const bool not_held = !w.a.is_held() && w.wait.is_empty();
        w.wait.immed(w.a, 7);                 // later someone wakes it (DECISION)
        run_until(w.wait, 7);
        check("set_next_inst on a node that is not held: nothing runs until it is started",
              not_held && w.log == "A");
    }

    // 12. immed jumps the queue, even ahead of overdue nodes
    {
        World w;
        w.wait.hold(w.a, 0, 10);
        w.wait.hold(w.b, 0, 20);
        w.wait.hold(w.c, 0, 999);
        w.wait.immed(w.c, 100);              // a and b are overdue at 100
        run_until(w.wait, 100);
        check("immed c at 100 with a, b overdue: c a b", w.log == "cab" && w.wait.is_empty());
    }

    // 13. One node per step: the main loop gets a turn between them
    {
        World w;
        w.wait.hold(w.a, 0, 10);
        w.wait.hold(w.b, 0, 10);
        w.wait.hold(w.c, 0, 10);
        step(w.wait, 10);
        const std::string first = w.log;
        step(w.wait, 10);
        check("three due nodes: one step runs one", first == "a" && w.log == "ab");
    }

    // 14. A periodic node that re-holds itself
    {
        World w;
        w.wait.holdc(w.period, 0, &write_and_repeat);
        for (std::uint32_t now = 0; now <= 100; ++now) run_until(w.wait, now);
        const std::string ticks = w.log;
        run_until(w.wait, 1000);             // a long stall
        check("period 10 over 100 ms: 10 runs; after a stall: 1 run, no endless loop",
              ticks == std::string(10, 'p') && w.log == std::string(11, 'p') && !w.wait.is_empty());
    }

    // 15. A handler that cancels another node that is also due
    {
        World w;
        w.wait.holdc(w.a, 0, 10, &cancel_b);
        w.wait.hold(w.b, 0, 10);
        run_until(w.wait, 10);
        check("handler cancels a due node: it never runs", w.log == "x" && w.wait.is_empty());
    }

    // 16. The clock wrapping round
    {
        World w;
        w.wait.hold(w.a, 0xFFFFFF00u, 0x200);   // due at 0x100, after the wrap
        w.wait.hold(w.b, 0xFFFFFF00u, 0x80);    // due at 0xFFFFFF80, before it
        const bool b_early = w.wait.is_node_ready(0xFFFFFF7Fu);
        run_until(w.wait, 0xFFFFFFFFu);
        const std::string before_wrap = w.log;
        const bool a_early = w.wait.is_node_ready(0xFFFFFFFFu) || w.wait.is_node_ready(0xFFu);
        run_until(w.wait, 0x100);
        check("wrap: b due before 0xFFFFFFFF runs then, a due at 0x100 runs after the wrap",
              !b_early && before_wrap == "b" && !a_early && w.log == "ba");
    }
    {
        World w;
        w.wait.hold(w.a, 0xFFFFF000u, 4096);    // due at exactly 0
        check("4096 ms from 0xFFFFF000: not ready at 0xFFFFFFFF, ready at 0",
              !w.wait.is_node_ready(0xFFFFFFFFu) && w.wait.is_node_ready(0));
    }

    // 17. Random holds, cancels and time steps, against the model
    {
        WaitList wait;
        std::array<NumberNode, Model::N> nodes;
        std::vector<std::size_t> fired;
        for (std::size_t i = 0; i < nodes.size(); ++i) { nodes[i].number = i; nodes[i].fired = &fired; }
        Model m;

        std::mt19937 rng(12345);
        std::uniform_int_distribution<std::size_t> pick(0, Model::N - 1);
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
                wait.hold(nodes[n], now, d);
                m.hold(n, now64 + d);
            } else if (o < 7) {              // cancel
                wait.delete_node(nodes[n]);
                m.cancel(n);
            } else {                         // time passes; run what is due, one per step
                now64 += advance(rng);
                const auto t = static_cast<std::uint32_t>(now64);
                for (;;) {
                    const int expect = m.due(now64);
                    const bool ready = wait.is_node_ready(t);
                    if (ready != (expect >= 0)) { same = false; break; }
                    if (!ready) break;
                    fired.clear();
                    step(wait, t);
                    if (fired.size() != 1 || fired[0] != static_cast<std::size_t>(expect)) { same = false; break; }
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
