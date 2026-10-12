// test_line_reader.cpp - tests of LineReader (line_reader.hpp), run on the PC
//
// 1. LineReader alone: feeds byte strings into a LineReader<8> and checks which
//    lines come out. '\n' ends a line and is never stored; every other byte,
//    including '\r', is kept, so "hi\r\n" gives "hi\r". N counts the trailing '\r'.
// 2. LineReader + unseal: the same lines as tests/test_line.py checks with the
//    Python unseal, so both sides accept exactly the same lines. Regression tests
//    for '\r' inserted into a line (it used to be skipped, hiding the damage).
//
// Build (from tests/):  g++-14 -std=c++23 -I.. -Wall -Wextra -Wconversion -D_GLIBCXX_ASSERTIONS
//             test_line_reader.cpp -o test_line_reader
// -D_GLIBCXX_ASSERTIONS makes every buf[] index checked, so a write past the end
// of the line buffer stops the test instead of silently corrupting the next member.
// Exit status: 0 if every check passed.
#include <cstdint>
#include <optional>
#include <print>
#include <string>
#include <string_view>
#include <vector>

#include "line_reader.hpp"
#include "seal.hpp"

using Lines = std::vector<std::string>;

static int failures = 0;

static void check(std::string_view name, bool ok) {
    std::println("{}  {}", ok ? "PASS" : "FAIL", name);
    failures += !ok;
}

// Feed every byte of input; return the lines handed out, in order.
template <std::size_t N>
static Lines feed_all(LineReader<N>& reader, std::string_view input) {
    Lines out;
    for (char c : input) {
        if (reader.feed(static_cast<std::uint8_t>(c))) out.emplace_back(reader.line());
    }
    return out;
}

// What the MCU does with one line from the wire: LineReader, then unseal.
// The body if the line is accepted, std::nullopt if it is rejected or never completed.
static std::optional<std::string> receive(std::string_view wire) {
    LineReader<64> reader;
    std::optional<std::string> body;
    for (char c : wire) {
        if (reader.feed(static_cast<std::uint8_t>(c))) {
            const auto b = unseal(reader.line());
            body = b ? std::optional<std::string>(*b) : std::nullopt;
        }
    }
    return body;
}

// The same table as LINE_CASES in tests/test_line.py: keep the two in sync.
struct LineCase {
    std::string_view name;
    std::string_view wire;
    bool accepted;
};

constexpr LineCase LINE_CASES[] = {
    {"a normal line",                        "2 0 5 hello*EBDC\r\n",      true},
    {"'\\n' without '\\r'",                  "2 0 5 hello*EBDC\n",        true},
    {"two '\\r' before '\\n'",               "2 0 5 hello*EBDC\r\r\n",    false},
    {"three '\\r' before '\\n'",             "2 0 5 hello*EBDC\r\r\r\n",  false},
    {"'\\r' before the line",                "\r2 0 5 hello*EBDC\r\n",    false},
    {"'\\r' inserted in the body",           "2 \r0 5 hello*EBDC\r\n",    false},
    {"two '\\r' inserted in the body",       "2 0 5 he\r\rllo*EBDC\r\n",  false},
    {"'\\r' inserted in the CRC",            "2 0 5 hello*EB\rDC\r\n",    false},
    {"a body byte replaced by '\\r'",        "2 \r 5 hello*EBDC\r\n",     false},
    {"only '\\r's",                          "\r\r\r\n",                  false},
};

int main() {
    // 1. LineReader alone
    {
        LineReader<8> r;
        check("\"hi\\r\\n\" gives \"hi\\r\" ('\\r' is kept, '\\n' is not)", feed_all(r, "hi\r\n") == Lines{"hi\r"});
    }
    {
        LineReader<8> r;
        check("'\\n' alone also ends a line", feed_all(r, "hi\n") == Lines{"hi"});
    }
    {
        LineReader<8> r;
        check("an empty line (\"\\n\") is ignored", feed_all(r, "\n\n").empty() && r.dropped() == 0);
    }
    {
        LineReader<8> r;
        check("\"\\r\\n\" alone gives \"\\r\" (unseal rejects it)", feed_all(r, "\r\n") == Lines{"\r"});
    }
    {
        LineReader<8> r;
        check("exactly N characters fit, the trailing '\\r' included",
              feed_all(r, "abcdefg\r\n") == Lines{"abcdefg\r"} && r.dropped() == 0);
    }
    {
        LineReader<8> r;
        check("N + 1 characters: the line is dropped and counted",
              feed_all(r, "abcdefgh\r\n").empty() && r.dropped() == 1);
    }
    {
        LineReader<8> r;
        check("after a too-long line, the next line arrives whole",
              feed_all(r, "abcdefghijklmnop\r\nok\r\n") == Lines{"ok\r"} && r.dropped() == 1);
    }
    {
        LineReader<8> r;
        const Lines a = feed_all(r, "ab");
        const Lines b = feed_all(r, "c\r\n");
        check("a line split across calls", a.empty() && b == Lines{"abc\r"});
    }
    {
        LineReader<8> r;
        check("several lines in one burst", feed_all(r, "a\nbb\r\nccc\n") == Lines{"a", "bb\r", "ccc"});
    }
    {
        LineReader<8> r;
        check("'*', spaces and inner '\\r' are kept", feed_all(r, "2 \r0*1A\r\n") == Lines{"2 \r0*1A\r"});
    }
    check("capacity() is N", LineReader<8>::capacity() == 8 && LineReader<64>::capacity() == 64);

    // 2. LineReader + unseal: accepted exactly when the Python unseal accepts
    for (const LineCase& c : LINE_CASES) {
        const auto body = receive(c.wire);
        const bool ok = c.accepted ? (body && *body == "2 0 5 hello") : !body;
        check(std::string("LineReader + unseal: ") + std::string(c.name) +
                  (c.accepted ? " -> accepted" : " -> rejected"),
              ok);
    }

    std::println("\n{}", failures ? "SOME TESTS FAILED" : "all passed");
    return failures != 0;
}
