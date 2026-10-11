// test_pipeline.cpp - seal.hpp + ring_buffer.hpp + line_reader.hpp together, on the PC.
//
//   RX path (Pi -> MCU):   sealed bytes -> RingBuffer.push(byte)       ("UART interrupt")
//                                       -> RingBuffer.pop -> LineReader.feed -> unseal  ("main loop")
//   TX path (MCU -> Pi):   seal -> RingBuffer.push(span)               ("main loop")
//                                -> waiting / free_bytes -> LineReader -> unseal  ("DMA + Pi")
//
// The threaded tests run the writer and the reader on two real threads, like an
// interrupt and the main loop, with small rings so they fill up and wrap often.
//
// Build (from tests/):  g++-14 -std=c++23 -I.. -O2 -Wall -Wextra -Wconversion -pthread
//             test_pipeline.cpp -o test_pipeline
// Exit status: 0 if every check passed.
#include <array>
#include <cstdint>
#include <print>
#include <string>
#include <thread>
#include <vector>

#include "line_reader.hpp"
#include "ring_buffer.hpp"
#include "seal.hpp"

static int failures = 0;

static void check(const char* name, bool ok) {
    std::println("{}  {}", ok ? "PASS" : "FAIL", name);
    failures += !ok;
}

// Seal body into a fresh buffer and return the whole line as a std::string.
static std::string sealed(std::string_view body) {
    std::array<char, 128> buf{};
    for (std::size_t i = 0; i < body.size(); ++i) buf[i] = body[i];
    return std::string(seal(buf, body.size()));
}

// The body of message number i: different lengths, digits, spaces and '*'.
static std::string body_of(std::uint32_t i) {
    return "2 " + std::to_string(i) + " 5 msg*" + std::string(i % 23, 'x');
}

// Everything the "main loop" got out of the RX path.
struct Received {
    std::vector<std::string> bodies;
    std::size_t bad = 0;        // lines that unseal() rejected
};

// The main loop's work for one byte: pop -> feed -> unseal.
template <std::size_t N>
static void receive(LineReader<N>& reader, std::uint8_t byte, Received& got) {
    if (!reader.feed(byte)) return;
    if (auto body = unseal(reader.line())) got.bodies.emplace_back(*body);
    else ++got.bad;
}

// RX path on one thread: push every byte of the input, then drain.
static Received rx_single_thread(std::string_view wire) {
    static RingBuffer<512> rx;
    LineReader<64> reader;
    Received got;
    std::uint8_t byte;
    for (char c : wire) {
        rx.push(static_cast<std::uint8_t>(c));
        while (rx.pop(byte)) receive(reader, byte, got);
    }
    return got;
}

int main() {
    // 1. Three good lines arrive and come out in order.
    {
        const std::string wire = sealed("2 0 5 hello") + sealed("7") + sealed("a*b*c");
        const Received got = rx_single_thread(wire);
        check("RX: 3 lines, in order",
              got.bodies == std::vector<std::string>{"2 0 5 hello", "7", "a*b*c"} && got.bad == 0);
    }

    // 2. One damaged byte: only that line is lost, its neighbours arrive.
    {
        std::string middle = sealed("2 1 5 world");
        middle[4] = 'W';
        const std::string wire = sealed("first") + middle + sealed("last");
        const Received got = rx_single_thread(wire);
        check("RX: a damaged line is rejected, neighbours survive",
              got.bodies == std::vector<std::string>{"first", "last"} && got.bad == 1);
    }

    // 3. A line longer than the LineReader is dropped, the next one arrives.
    {
        const std::string wire = sealed("ok1") + std::string(200, 'z') + "\r\n" + sealed("ok2");
        const Received got = rx_single_thread(wire);
        check("RX: an over-long line is dropped, the reader resynchronises",
              got.bodies == std::vector<std::string>{"ok1", "ok2"} && got.bad == 0);
    }

    // 4. RX with two threads: the "interrupt" pushes byte by byte into a small ring
    //    (retrying when it's full), the "main loop" pops, splits and unseals.
    {
        constexpr std::uint32_t COUNT = 200'000;
        static RingBuffer<64> rx;
        std::thread interrupt([] {
            for (std::uint32_t i = 0; i < COUNT; ++i) {
                for (char c : sealed(body_of(i))) {
                    while (!rx.push(static_cast<std::uint8_t>(c))) {}   // full: wait
                }
            }
        });
        LineReader<64> reader;
        Received got;
        std::uint8_t byte;
        while (got.bodies.size() + got.bad < COUNT) {
            if (rx.pop(byte)) receive(reader, byte, got);
        }
        interrupt.join();

        bool in_order = got.bodies.size() == COUNT;
        for (std::uint32_t i = 0; in_order && i < COUNT; ++i) in_order = got.bodies[i] == body_of(i);
        check("RX threaded: 200 000 lines through RingBuffer<64>, all intact and in order",
              in_order && got.bad == 0);
    }

    // 5. TX with two threads: the "main loop" seals each line and pushes it whole;
    //    the "DMA" sends contiguous blocks with waiting()/free_bytes(); on the far
    //    side the "Pi" splits and unseals them.
    {
        constexpr std::uint32_t COUNT = 200'000;
        static RingBuffer<256> tx;
        Received got;
        std::thread dma([&got] {
            LineReader<64> pi_reader;
            while (got.bodies.size() + got.bad < COUNT) {
                const auto block = tx.waiting();
                for (std::uint8_t b : block) receive(pi_reader, b, got);
                if (!block.empty()) (void)tx.free_bytes(block.size());
            }
        });
        for (std::uint32_t i = 0; i < COUNT; ++i) {
            std::array<char, 128> buf{};
            const std::string body = body_of(i);
            for (std::size_t k = 0; k < body.size(); ++k) buf[k] = body[k];
            const std::string_view line = seal(buf, body.size());
            const std::span<const std::uint8_t> bytes{
                reinterpret_cast<const std::uint8_t*>(line.data()), line.size()};
            while (!tx.push(bytes)) {}                                  // full: wait
        }
        dma.join();

        bool in_order = got.bodies.size() == COUNT;
        for (std::uint32_t i = 0; in_order && i < COUNT; ++i) in_order = got.bodies[i] == body_of(i);
        check("TX threaded: 200 000 lines through RingBuffer<256> in DMA-sized blocks, all intact",
              in_order && got.bad == 0);
    }

    std::println("\n{}", failures ? "SOME TESTS FAILED" : "all passed");
    return failures != 0;
}
