// test_circle_of_buffers.cpp - tests of CircleOfBuffers<T, N> (circle_of_buffers.hpp), run on the PC
//
//   empty:       start_operation() gives nullptr, no red buffers
//   assign:      gives the same (yellow) buffer until release(); release() makes it red
//   full:        N distinct buffers of the circle, then assign() gives nullptr
//   FIFO:        start_operation() gives the red buffers in the order they were released
//   device:      a second start_operation() is refused while the device has a buffer;
//                cancel_operation() keeps it the oldest red one; operation_is_completed()
//                turns it green and frees the device
//   reuse:       a buffer given again keeps its previous contents (the caller sets them)
//   wrap:        N = 1, 2, 3, 4, 5 (not only powers of 2), many times around the circle
//   random:      200 000 random operations compared with a model (a queue of values)
//   misuse:      release() with every buffer red, cancel_operation() and
//                operation_is_completed() with no buffer in the device abort with the
//                asserts on (checked in a child process)
//   threads:     a producer thread and a device thread (the DMA and its interrupt),
//                500 000 buffers: every one arrives once, in order, never torn. Run it
//                with -fsanitize=thread too.
//   two consumers: the MCU's consumer side runs in two contexts, main (its kick
//                after release) and the TX-complete ISR (complete, then kick the next
//                one). They never run at the same time: the ISR comes only while the
//                device has a buffer, and main gets nullptr then. Two threads could
//                overlap, so here they take turns with a token, like main and the ISR.
//                The token hands main -> ISR with release/acquire (on the MCU the
//                hardware orders that: the interrupt comes after the DMA start), but
//                ISR -> main relaxed: there only the circle's own atomics may order
//                the ISR's NEXTR before main's next start_operation(). TSan checks it.
//
// Build (from tests/):  g++-14 -std=c++23 -I.. -O2 -Wall -Wextra -Wshadow -Wconversion -Wsign-conversion
//             -pthread test_circle_of_buffers.cpp -o test_circle_of_buffers
// Exit status: 0 if every check passed.
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <csignal>
#include <cstdio>
#include <deque>
#include <print>
#include <random>
#include <set>
#include <thread>

#include <sys/wait.h>
#include <unistd.h>

#include "circle_of_buffers.hpp"

static int failures = 0;

static void check(const char* name, bool ok) {
    std::println("{}  {}", ok ? "PASS" : "FAIL", name);
    failures += !ok;
}

// Runs f in a child process: true if the child dies by abort() (a failed assert).
// The child's stderr goes to /dev/null, so the assert message does not clutter the log.
template <class F>
static bool aborts(F f) {
    std::fflush(stdout);
    const pid_t pid = fork();
    if (pid == 0) {
        if (std::freopen("/dev/null", "w", stderr) == nullptr) _exit(2);
        f();
        _exit(0);                           // no abort: the misuse went through
    }
    int status = 0;
    waitpid(pid, &status, 0);
    return WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT;
}

// A buffer like the UART's: bytes and how many are filled
struct Line {
    std::array<std::uint8_t, 16> data{};
    std::size_t len = 0;
};

// Puts value into the next buffer: false if the circle is full
template <std::size_t N>
static bool put(CircleOfBuffers<int, N>& c, int value) {
    int* b = c.assign();
    if (b == nullptr) return false;
    *b = value;
    c.release();
    return true;
}

// Takes the oldest red buffer through the device: false if none
template <std::size_t N>
static bool take(CircleOfBuffers<int, N>& c, int& value) {
    int* b = c.start_operation();
    if (b == nullptr) return false;
    value = *b;
    c.operation_is_completed();
    return true;
}

// Fills and empties the circle `rounds` times around: true if every value comes out in order
template <std::size_t N>
static bool wraps(std::size_t rounds) {
    CircleOfBuffers<int, N> c;
    int next_in = 0;
    int next_out = 0;
    for (std::size_t r = 0; r < rounds; ++r) {
        while (put(c, next_in)) ++next_in;                  // fill: exactly N
        if (c.red_buffers_quantity() != N) return false;
        int v = 0;
        for (std::size_t k = 0; k < r % N + 1; ++k) {       // empty a few, not always all
            if (!take(c, v) || v != next_out) return false;
            ++next_out;
        }
    }
    int v = 0;
    while (take(c, v)) {
        if (v != next_out) return false;
        ++next_out;
    }
    return next_out == next_in && c.red_buffers_quantity() == 0;
}

// Random operations against a model: the queue of red values, the yellow and device states
template <std::size_t N>
static bool random_against_model(std::uint32_t seed, std::size_t steps) {
    CircleOfBuffers<int, N> c;
    std::deque<int> red;                    // the model: released values, oldest first
    int* yellow = nullptr;                  // the buffer from assign(), not yet released
    int* device = nullptr;                  // the buffer from start_operation()
    int value = 0;
    std::mt19937 rng(seed);
    for (std::size_t s = 0; s < steps; ++s) {
        switch (rng() % 5) {
        case 0: {                           // assign
            int* b = c.assign();
            if (red.size() == N) { if (b != nullptr) return false; break; }
            if (b == nullptr || (yellow != nullptr && b != yellow) || b == device) return false;
            yellow = b;
            *b = value;
            break;
        }
        case 1:                             // release the yellow buffer
            if (yellow == nullptr) break;
            c.release();
            red.push_back(value++);
            yellow = nullptr;
            break;
        case 2: {                           // start
            int* b = c.start_operation();
            if (device != nullptr || red.empty()) { if (b != nullptr) return false; break; }
            if (b == nullptr || *b != red.front()) return false;
            device = b;
            break;
        }
        case 3:                             // the device refuses it
            if (device == nullptr) break;
            c.cancel_operation();
            device = nullptr;
            break;
        case 4:                             // the device is done with it
            if (device == nullptr) break;
            if (*device != red.front()) return false;      // nobody wrote into it meanwhile
            c.operation_is_completed();
            red.pop_front();
            device = nullptr;
            break;
        }
        if (c.red_buffers_quantity() != red.size()) return false;
    }
    return true;
}

// A buffer for the thread test: every field derives from seq, so a torn one shows
struct Packet {
    std::uint64_t seq = 0;
    std::array<std::uint64_t, 7> body{};
};

static void fill(Packet& p, std::uint64_t seq) {
    p.seq = seq;
    for (std::size_t i = 0; i < p.body.size(); ++i) p.body[i] = seq * 31 + i;
}

static bool whole(const Packet& p, std::uint64_t seq) {
    if (p.seq != seq) return false;
    for (std::size_t i = 0; i < p.body.size(); ++i)
        if (p.body[i] != seq * 31 + i) return false;
    return true;
}

int main() {
    // 1. Empty
    {
        CircleOfBuffers<int, 4> c;
        check("empty: start_operation() gives nullptr, no red buffers",
              c.start_operation() == nullptr && c.red_buffers_quantity() == 0);
    }

    // 2. assign() gives the same yellow buffer until release()
    {
        CircleOfBuffers<int, 4> c;
        int* a = c.assign();
        int* b = c.assign();
        check("assign() twice before release(): the same buffer, still no red one",
              a != nullptr && a == b && c.red_buffers_quantity() == 0 && c.start_operation() == nullptr);
        *a = 7;
        c.release();
        int* d = c.assign();
        check("after release(): one red buffer, assign() gives the next one",
              c.red_buffers_quantity() == 1 && d != nullptr && d != a);
    }

    // 3. Full: N distinct buffers, then nullptr
    {
        constexpr std::size_t N = 4;
        CircleOfBuffers<int, N> c;
        std::set<int*> seen;
        for (std::size_t i = 0; i < N; ++i) {
            int* b = c.assign();
            if (b != nullptr) seen.insert(b);
            c.release();
        }
        check("4 assign/release give 4 different buffers", seen.size() == N);
        check("then the circle is full: assign() gives nullptr, 4 red",
              c.assign() == nullptr && c.red_buffers_quantity() == N);
        int* first = *seen.begin();
        int* last = *seen.rbegin();
        check("the 4 buffers are one array, one int apart", last - first == static_cast<std::ptrdiff_t>(N - 1));
    }

    // 4. FIFO and the device
    {
        CircleOfBuffers<int, 4> c;
        for (int v = 10; v < 14; ++v) put(c, v);
        int* b = c.start_operation();
        check("start_operation() gives the oldest red buffer", b != nullptr && *b == 10);
        check("a second start_operation() while the device has it: nullptr",
              c.start_operation() == nullptr);
        check("the buffer in the device is still red, the circle still full",
              c.red_buffers_quantity() == 4 && c.assign() == nullptr);
        c.operation_is_completed();
        check("operation_is_completed(): 3 red, assign() works again",
              c.red_buffers_quantity() == 3 && c.assign() != nullptr);
        int v = 0;
        bool order = true;
        for (int want = 11; want < 14; ++want) order = order && take(c, v) && v == want;
        check("the rest come out in release order: 11, 12, 13", order && c.red_buffers_quantity() == 0);
    }

    // 5. cancel_operation(): the buffer stays the oldest red one
    {
        CircleOfBuffers<int, 4> c;
        put(c, 1);
        put(c, 2);
        int* b = c.start_operation();
        c.cancel_operation();
        check("after cancel_operation(): still 2 red", c.red_buffers_quantity() == 2);
        int* again = c.start_operation();
        check("start_operation() gives the same buffer again", again == b && again != nullptr && *again == 1);
        c.operation_is_completed();
        int v = 0;
        check("then the next one, 2", take(c, v) && v == 2);
    }

    // 6. A reused buffer keeps its previous contents
    {
        CircleOfBuffers<Line, 1> c;
        Line* a = c.assign();
        a->data[0] = 'A';
        a->len = 5;
        c.release();
        (void)c.start_operation();
        c.operation_is_completed();
        Line* b = c.assign();
        check("N = 1: the same buffer again, with the old contents (the caller sets them)",
              b == a && b->data[0] == 'A' && b->len == 5);
    }

    // 7. Around the circle many times, any N
    check("N = 1: 1000 times around, every value in order", wraps<1>(1000));
    check("N = 2: 1000 times around, every value in order", wraps<2>(1000));
    check("N = 3: 1000 times around, every value in order (not a power of 2)", wraps<3>(1000));
    check("N = 4: 1000 times around, every value in order", wraps<4>(1000));
    check("N = 5: 1000 times around, every value in order (not a power of 2)", wraps<5>(1000));

    // 8. Random operations against a model
    check("N = 3: 200 000 random assign/release/start/cancel/complete match the model",
          random_against_model<3>(1, 200'000));
    check("N = 4: 200 000 random operations match the model", random_against_model<4>(2, 200'000));
    check("N = 1: 200 000 random operations match the model", random_against_model<1>(3, 200'000));

#ifndef NDEBUG
    // 9. Misuse stops at once with the asserts on
    check("release() with every buffer red (no successful assign) aborts", aborts([] {
        CircleOfBuffers<int, 2> c;
        put(c, 1);
        put(c, 2);
        c.release();
    }));
    check("operation_is_completed() with no buffer in the device aborts", aborts([] {
        CircleOfBuffers<int, 2> c;
        put(c, 1);
        c.operation_is_completed();
    }));
    check("cancel_operation() with no buffer in the device aborts", aborts([] {
        CircleOfBuffers<int, 2> c;
        put(c, 1);
        c.cancel_operation();
    }));
#endif

    // 10. Two threads: a producer and a device (the DMA with its interrupt)
    {
        constexpr std::uint64_t COUNT = 500'000;
        CircleOfBuffers<Packet, 4> c;

        std::thread producer([&] {
            for (std::uint64_t seq = 0; seq < COUNT;) {
                Packet* p = c.assign();
                if (p == nullptr) { std::this_thread::yield(); continue; }   // full: try again
                fill(*p, seq++);
                c.release();
            }
        });

        bool in_order = true;
        std::uint64_t expected = 0;
        std::uint64_t cancels = 0;
        while (expected < COUNT) {
            Packet* p = c.start_operation();
            if (p == nullptr) { std::this_thread::yield(); continue; }       // nothing red yet
            if (expected % 97 == 0 && cancels < expected / 97 + 1) {         // now and then the device refuses
                ++cancels;
                c.cancel_operation();
                continue;
            }
            in_order = in_order && whole(*p, expected);
            ++expected;
            c.operation_is_completed();
        }
        producer.join();

        check("500 000 buffers through two threads: each once, in order, never torn",
              in_order && expected == COUNT);
        check("with cancelled starts among them, and the circle empty at the end",
              cancels > 0 && c.red_buffers_quantity() == 0 && c.start_operation() == nullptr);
    }

    // 11. Two consumer contexts taking turns: main's kick and the TX-complete ISR
    {
        constexpr std::uint64_t COUNT = 300'000;
        CircleOfBuffers<Packet, 4> c;
        std::atomic<int> turn = 0;                      // 0: main may kick, 1: the device is busy (the ISR's)
        std::atomic<std::uint64_t> sent = 0;            // buffers started so far (the DMA reads them)
        std::atomic<bool> in_order = true;
        std::atomic<std::uint64_t> by_main = 0;
        std::atomic<std::uint64_t> by_isr = 0;

        // The DMA reads the buffer it was started on
        auto dma_reads = [&](const Packet& p) {
            const std::uint64_t seq = sent.load(std::memory_order_relaxed);
            if (!whole(p, seq)) in_order.store(false, std::memory_order_relaxed);
            sent.store(seq + 1, std::memory_order_relaxed);
        };

        std::thread producer([&] {
            for (std::uint64_t seq = 0; seq < COUNT;) {
                Packet* p = c.assign();
                if (p == nullptr) { std::this_thread::yield(); continue; }
                fill(*p, seq++);
                c.release();
            }
        });

        // The ISR: comes only while the device has a buffer
        std::thread isr([&] {
            for (;;) {
                if (turn.load(std::memory_order_acquire) != 1) {          // no transfer running
                    if (sent.load(std::memory_order_relaxed) == COUNT) return;
                    std::this_thread::yield();
                    continue;
                }
                c.operation_is_completed();                               // the transfer ended
                if (Packet* p = c.start_operation()) {                    // its kick: the next one
                    dma_reads(*p);
                    by_isr.fetch_add(1, std::memory_order_relaxed);
                    continue;                                             // the device is busy again
                }
                turn.store(0, std::memory_order_relaxed);                 // idle: only the circle orders this
            }
        });

        // main: kicks only while the device is idle
        while (sent.load(std::memory_order_relaxed) < COUNT) {
            if (turn.load(std::memory_order_relaxed) != 0) { std::this_thread::yield(); continue; }
            Packet* p = c.start_operation();
            if (p == nullptr) { std::this_thread::yield(); continue; }    // nothing red yet
            dma_reads(*p);
            by_main.fetch_add(1, std::memory_order_relaxed);
            turn.store(1, std::memory_order_release);                     // the interrupt comes after the start
        }
        producer.join();
        isr.join();

        check("300 000 buffers started by main or by the ISR in turn: each once, in order, never torn",
              in_order.load() && sent.load() == COUNT);
        check("both contexts started buffers, and the circle is empty at the end",
              by_main.load() > 0 && by_isr.load() > 0 && c.red_buffers_quantity() == 0);
    }

    std::println("\n{}", failures ? "SOME TESTS FAILED" : "all passed");
    return failures != 0;
}
