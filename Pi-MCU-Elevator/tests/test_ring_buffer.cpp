// test_ring_buffer.cpp - tests of RingBuffer (ring_buffer.hpp), run on the PC
//
//   single thread:  empty, full, drops, order, wrap-around, push(span),
//                   waiting() + free_bytes(), clear_buffer()
//   two threads:    RX use (push(byte) / pop) and TX use (push(span) /
//                   waiting() + free_bytes()), many bytes through small rings,
//                   like an interrupt and the main loop
//
// Build (from tests/):  g++-14 -std=c++23 -I.. -O2 -Wall -Wextra -Wconversion -pthread
//             test_ring_buffer.cpp -o test_ring_buffer
// Data races: build with -fsanitize=thread instead of -O2.
// Exit status: 0 if every check passed.
#include <array>
#include <cstdint>
#include <print>
#include <thread>

#include "ring_buffer.hpp"

static int failures = 0;

static void check(const char* name, bool ok) {
    std::println("{}  {}", ok ? "PASS" : "FAIL", name);
    failures += !ok;
}

int main() {
    std::uint8_t byte = 0xAA;

    // 1. Empty, full, drops and order with RingBuffer<8>.
    {
        RingBuffer<8> rb;
        check("new buffer: pop fails, byte unchanged", !rb.pop(byte) && byte == 0xAA && rb.size() == 0);

        bool all_in = true;
        for (std::uint8_t i = 0; i < 8; ++i) all_in &= rb.push(i);
        check("8 bytes fit in RingBuffer<8>", all_in && rb.size() == 8);

        check("a 9th byte is refused and counted", !rb.push(99) && rb.dropped() == 1);

        bool in_order = true;
        for (std::uint8_t i = 0; i < 8; ++i) in_order &= rb.pop(byte) && byte == i;
        check("they come out in order, then empty", in_order && !rb.pop(byte) && rb.size() == 0);
    }

    // 2. Wrap-around: 5 in, 5 out, 1000 times round the 8 cells.
    {
        RingBuffer<8> rb;
        std::uint8_t next_in = 0, next_out = 0;
        bool ok = true;
        for (int round = 0; round < 1000; ++round) {
            for (int k = 0; k < 5; ++k) ok &= rb.push(next_in++);
            for (int k = 0; k < 5; ++k) ok &= rb.pop(byte) && byte == next_out++;
        }
        check("wrap-around keeps the order", ok && rb.dropped() == 0);
    }

    // 3. push(span) is all or nothing.
    {
        RingBuffer<8> rb;
        const std::array<std::uint8_t, 5> a{1, 2, 3, 4, 5};
        const std::array<std::uint8_t, 4> b{6, 7, 8, 9};
        const bool first = rb.push(std::span<const std::uint8_t>(a));
        const bool second = rb.push(std::span<const std::uint8_t>(b));    // only 3 free
        check("push(span): a block that doesn't fit is refused whole",
              first && !second && rb.size() == 5 && rb.dropped() == 4);
    }

    // 4. waiting() + free_bytes(): A B C D E wrapped round the end (cells 6, 7, 0, 1, 2).
    {
        RingBuffer<8> rb;
        for (int i = 0; i < 6; ++i) { rb.push(0); rb.pop(byte); }       // move both indices to 6
        const std::array<std::uint8_t, 5> abcde{'A', 'B', 'C', 'D', 'E'};
        rb.push(std::span<const std::uint8_t>(abcde));

        auto w = rb.waiting();
        const bool first = w.size() == 2 && w[0] == 'A' && w[1] == 'B';  // up to the end of the array
        const bool freed = rb.free_bytes(w.size());
        w = rb.waiting();
        const bool second = w.size() == 3 && w[0] == 'C' && w[2] == 'E'; // the rest, from the start
        check("waiting(): a wrapped block comes out in two contiguous parts", first && freed && second);

        check("free_bytes() refuses more than is waiting", !rb.free_bytes(4) && rb.size() == 3);
        check("free_bytes() can free everything", rb.free_bytes(3) && rb.waiting().empty());
    }

    // 5. clear_buffer() discards everything waiting, and the buffer still works after.
    {
        RingBuffer<8> rb;
        for (std::uint8_t i = 0; i < 5; ++i) rb.push(i);
        rb.clear_buffer();
        const bool empty = rb.size() == 0 && !rb.pop(byte);
        const bool works = rb.push(42) && rb.pop(byte) && byte == 42;
        check("clear_buffer() empties the buffer; it keeps working", empty && works);
    }

    // 6. RX use on two threads: the "interrupt" pushes byte by byte, retrying when full;
    //    the "main loop" pops. Every byte must arrive, in order.
    {
        constexpr std::uint32_t COUNT = 20'000'000;
        static RingBuffer<64> rx;
        std::thread interrupt([] {
            for (std::uint32_t i = 0; i < COUNT;) {
                if (rx.push(static_cast<std::uint8_t>(i))) ++i;
            }
        });
        std::uint32_t wrong = 0;
        for (std::uint32_t i = 0; i < COUNT;) {
            if (rx.pop(byte)) {
                wrong += byte != static_cast<std::uint8_t>(i);
                ++i;
            }
        }
        interrupt.join();
        check("2 threads, push(byte)/pop: 20 million bytes through RingBuffer<64>, in order", wrong == 0);
    }

    // 7. TX use on two threads: the "main loop" pushes 7-byte blocks; the "DMA" drains
    //    contiguous blocks with waiting() + free_bytes().
    {
        constexpr std::uint32_t COUNT = 7 * 1'000'000;
        static RingBuffer<64> tx;
        std::thread dma([] {
            std::uint32_t got = 0, wrong = 0;
            while (got < COUNT) {
                const auto block = tx.waiting();
                for (std::uint8_t b : block) wrong += b != static_cast<std::uint8_t>(got++);
                if (!block.empty()) (void)tx.free_bytes(block.size());
            }
            check("2 threads, push(span)/waiting/free_bytes: 7 million bytes, in order", wrong == 0);
        });
        std::array<std::uint8_t, 7> line{};
        for (std::uint32_t i = 0; i < COUNT; i += 7) {
            for (std::uint32_t k = 0; k < 7; ++k) line[k] = static_cast<std::uint8_t>(i + k);
            while (!tx.push(std::span<const std::uint8_t>(line))) {}     // full: wait
        }
        dma.join();
    }

    std::println("\n{}", failures ? "SOME TESTS FAILED" : "all passed");
    return failures != 0;
}
