// test_kernel.cpp - tests of Kernel (kernel.hpp) with small programs, run on the PC
//
//   the kernel:  a new kernel has nothing to do; start runs a program at once;
//                step runs one due node and says so; get_next_time follows the list
//   programs:    a blinker (two steps that hold each other), a periodic counter,
//                two programs side by side, one program sending a message to
//                another (start of its node), a supervisor that stops a program
//   time:        a program running across the 32-bit clock wrap; the same programs
//                driven by "jump to get_next_time" (event-driven, like the elevator's
//                CYCLE) give the same log as "every millisecond" (a real clock)
//
// A program here writes "time:name what" into a log, so a run is a string.
//
// Build (from tests/):  g++-14 -std=c++23 -I.. -O2 -Wall -Wextra -Wshadow -Wconversion -Wsign-conversion
//             test_kernel.cpp -o test_kernel
// Exit status: 0 if every check passed.
#include <cstdint>
#include <format>
#include <print>
#include <string>

#include "kernel.hpp"

static int failures = 0;

static void check(const char* name, bool ok) {
    std::println("{}  {}", ok ? "PASS" : "FAIL", name);
    failures += !ok;
}

// ---- A test program: a node with its kernel, its log, its name and a few params ----

struct Program : WaitNode {
    Program(Kernel& k, std::string& l, const char* n, NextInst first, std::uint32_t delay = 0) noexcept
        : WaitNode(first, delay), kernel(&k), log(&l), name(n) {}

    Kernel* kernel;
    std::string* log;
    const char* name;
    int count = 0;                 // how many times it ran (the counter's state)
    Program* other = nullptr;      // a program it talks to (messages, stop)
};

static Program& me(WaitNode& n) noexcept { return static_cast<Program&>(n); }

static void note(Program& p, std::uint32_t now, const char* what) {
    *p.log += std::format("{}:{} {}, ", now, p.name, what);
}

// The blinker: on for 100 ms, off for 900 ms; each step holds the other one.
static void blink_on(WaitNode& n, std::uint32_t now) noexcept;
static void blink_off(WaitNode& n, std::uint32_t now) noexcept {
    note(me(n), now, "off");
    me(n).kernel->holdc(n, now, 900, &blink_on);
}
static void blink_on(WaitNode& n, std::uint32_t now) noexcept {
    note(me(n), now, "on");
    me(n).kernel->holdc(n, now, 100, &blink_off);
}

// The counter: one step that holds itself again with the node's own DELAY.
static void count_up(WaitNode& n, std::uint32_t now) noexcept {
    ++me(n).count;
    note(me(n), now, "tick");
    me(n).kernel->hold(n, now);
}

// A sender: every 250 ms it starts its partner's node (a message), then waits again.
static void send(WaitNode& n, std::uint32_t now) noexcept {
    note(me(n), now, "send");
    me(n).kernel->start(*me(n).other, now);
    me(n).kernel->hold(n, now, 250);
}

// A receiver: runs only when someone starts it.
static void receive(WaitNode& n, std::uint32_t now) noexcept {
    ++me(n).count;
    note(me(n), now, "got");
}

// A supervisor: stops its partner.
static void stop_other(WaitNode& n, std::uint32_t now) noexcept {
    note(me(n), now, "stop");
    me(n).kernel->cancel(*me(n).other);
}

// Run the kernel with a real clock: every millisecond from `from` to `to`, all due steps.
static void every_ms(Kernel& k, std::uint32_t from, std::uint32_t to) noexcept {
    for (std::uint32_t now = from; ; ++now) {
        while (k.step(now)) {}
        if (now == to) break;
    }
}

// Run the kernel event-driven: jump straight to the next deadline, until `to`.
static void jump(Kernel& k, std::uint32_t to) noexcept {
    while (const auto t = k.get_next_time()) {
        if (WaitList::is_less(to, *t)) break;
        while (k.step(*t)) {}
    }
}

int main() {
    // 1. A new kernel
    {
        Kernel k;
        check("new kernel: step runs nothing, get_next_time is empty", !k.step(0) && !k.get_next_time());
    }

    // 2. start runs a program at once, and step reports it
    {
        Kernel k; std::string log;
        Program r{k, log, "r", &receive};
        k.start(r, 50);
        const bool early = k.step(49);
        const bool ran = k.step(50);
        const bool again = k.step(50);
        check("start at 50: not at 49, runs at 50, step says true once",
              !early && ran && !again && log == "50:r got, ");
    }

    // 3. The blinker
    {
        Kernel k; std::string log;
        Program b{k, log, "b", &blink_on};
        k.start(b, 0);
        every_ms(k, 0, 2100);
        check("blinker: on 0, off 100, on 1000, off 1100, on 2000, off 2100",
              log == "0:b on, 100:b off, 1000:b on, 1100:b off, 2000:b on, 2100:b off, ");
    }

    // 4. get_next_time follows the list
    {
        Kernel k; std::string log;
        Program b{k, log, "b", &blink_on};
        k.start(b, 0);
        const auto t0 = k.get_next_time();
        k.step(0);
        const auto t1 = k.get_next_time();
        k.step(100);
        const auto t2 = k.get_next_time();
        k.cancel(b);
        check("get_next_time: 0 after start, 100 after on, 1000 after off, empty after cancel",
              t0 == 0u && t1 == 100u && t2 == 1000u && !k.get_next_time());
    }

    // 5. cancel stops a program for good
    {
        Kernel k; std::string log;
        Program b{k, log, "b", &blink_on};
        k.start(b, 0);
        every_ms(k, 0, 500);
        k.cancel(b);
        every_ms(k, 501, 5000);
        check("cancel at 500: nothing after it", log == "0:b on, 100:b off, ");
    }

    // 6. One node per step
    {
        Kernel k; std::string log;
        Program x{k, log, "x", &receive}, y{k, log, "y", &receive};
        k.hold(x, 0, 10);
        k.hold(y, 0, 10);
        const bool one = k.step(10);
        const std::string after_one = log;
        const bool two = k.step(10);
        const bool three = k.step(10);
        check("two nodes due: two steps run one each, the third runs none",
              one && after_one == "10:x got, " && two && !three);
    }

    // 7. Two programs side by side
    {
        Kernel k; std::string log;
        Program b{k, log, "b", &blink_on};
        Program c{k, log, "c", &count_up, 300};
        k.start(b, 0);
        k.hold(c, 0);                        // the counter's first tick after its DELAY
        every_ms(k, 0, 1000);
        check("blinker + counter (300 ms): steps in time order",
              log == "0:b on, 100:b off, 300:c tick, 600:c tick, 900:c tick, 1000:b on, " && c.count == 3);
    }

    // 8. A message: one program starts another
    {
        Kernel k; std::string log;
        Program s{k, log, "s", &send}, r{k, log, "r", &receive};
        s.other = &r;
        k.hold(s, 0, 250);
        every_ms(k, 0, 1000);
        check("sender every 250 ms starts the receiver: it runs at once, 4 times",
              log == "250:s send, 250:r got, 500:s send, 500:r got, 750:s send, 750:r got, "
                     "1000:s send, 1000:r got, " && r.count == 4);
    }
    {
        Kernel k; std::string log;
        Program s{k, log, "s", &send}, r{k, log, "r", &receive}, late{k, log, "late", &receive};
        s.other = &r;
        k.hold(s, 0, 50);                    // the sender is due first
        k.hold(late, 0, 100);                // another node, also overdue by 200
        while (k.step(200)) {}               // the loop was busy until 200
        check("a message jumps ahead of an overdue node (start = immed): s r late",
              log == "200:s send, 200:r got, 200:late got, ");
    }

    // 9. A supervisor stops another program
    {
        Kernel k; std::string log;
        Program b{k, log, "b", &blink_on}, sup{k, log, "sup", &stop_other};
        sup.other = &b;
        k.start(b, 0);
        k.hold(sup, 0, 1050);
        every_ms(k, 0, 5000);
        check("supervisor stops the blinker at 1050",
              log == "0:b on, 100:b off, 1000:b on, 1050:sup stop, " && !k.get_next_time());
    }

    // 10. start on a program that is already waiting moves it to now
    {
        Kernel k; std::string log;
        Program r{k, log, "r", &receive};
        k.hold(r, 0, 1000);
        k.start(r, 10);
        every_ms(k, 0, 2000);
        check("start on a waiting node: runs now, not again later", log == "10:r got, ");
    }

    // 11. Across the 32-bit clock wrap
    {
        Kernel k; std::string log;
        Program b{k, log, "b", &blink_on};
        k.start(b, 0xFFFFFF00u);             // 256 ms before the wrap
        every_ms(k, 0xFFFFFF00u, 0x400u);
        check("blinker across the wrap: on 0xFFFFFF00, off 100 later, on 744 (= 1000 after start), off 844",
              log == "4294967040:b on, 4294967140:b off, 744:b on, 844:b off, ");
    }

    // 12. Event-driven time (jump to get_next_time) gives the same log as a real clock
    {
        auto run = [](bool event_driven) {
            Kernel k; std::string log;
            Program b{k, log, "b", &blink_on};
            Program c{k, log, "c", &count_up, 300};
            Program s{k, log, "s", &send}, r{k, log, "r", &receive};
            s.other = &r;
            k.start(b, 0);
            k.hold(c, 0);
            k.hold(s, 0, 250);
            if (event_driven) jump(k, 5000); else every_ms(k, 0, 5000);
            return log;
        };
        const std::string clock = run(false);
        const std::string events = run(true);
        check("jump to get_next_time (like CYCLE) = every millisecond (like SysTick)",
              !clock.empty() && clock == events);
    }

    std::println("\n{}", failures ? "SOME TESTS FAILED" : "all passed");
    return failures != 0;
}
