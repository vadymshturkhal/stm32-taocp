// test_line_reader.cpp - tests of LineReader (line_reader.hpp), run on the PC
//
// Feeds byte strings into a LineReader<8> and checks which lines come out:
// line endings, empty lines, the exact length limit, too-long lines and
// resynchronisation, lines split across calls, and bursts of lines.
//
// Build (from tests/):  g++-14 -std=c++23 -I.. -Wall -Wextra -Wconversion -D_GLIBCXX_ASSERTIONS
//             test_line_reader.cpp -o test_line_reader
// -D_GLIBCXX_ASSERTIONS makes every buf[] index checked, so a write past the end
// of the line buffer stops the test instead of silently corrupting the next member.
// Exit status: 0 if every check passed.
#include <cstdint>
#include <print>
#include <string>
#include <string_view>
#include <vector>

#include "line_reader.hpp"

using Lines = std::vector<std::string>;

static int failures = 0;

static void check(const char* name, bool ok) {
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

int main() {
    {
        LineReader<8> r;
        check("\"hi\\r\\n\" gives \"hi\"", feed_all(r, "hi\r\n") == Lines{"hi"});
    }
    {
        LineReader<8> r;
        check("'\\n' alone also ends a line", feed_all(r, "hi\n") == Lines{"hi"});
    }
    {
        LineReader<8> r;
        check("empty lines are ignored", feed_all(r, "\r\n\n\r\n").empty() && r.dropped() == 0);
    }
    {
        LineReader<8> r;
        check("exactly N characters fit", feed_all(r, "abcdefgh\n") == Lines{"abcdefgh"} && r.dropped() == 0);
    }
    {
        LineReader<8> r;
        check("N + 1 characters: the line is dropped and counted",
              feed_all(r, "abcdefghi\n").empty() && r.dropped() == 1);
    }
    {
        LineReader<8> r;
        check("after a too-long line, the next line arrives whole",
              feed_all(r, "abcdefghijklmnop\nok\n") == Lines{"ok"} && r.dropped() == 1);
    }
    {
        LineReader<8> r;
        const Lines a = feed_all(r, "ab");
        const Lines b = feed_all(r, "c\r\n");
        check("a line split across calls", a.empty() && b == Lines{"abc"});
    }
    {
        LineReader<8> r;
        check("several lines in one burst", feed_all(r, "a\nbb\r\nccc\n") == Lines{"a", "bb", "ccc"});
    }
    {
        LineReader<8> r;
        check("'*' and spaces are kept", feed_all(r, "2 0*1A\r\n") == Lines{"2 0*1A"});
    }
    check("capacity() is N", LineReader<8>::capacity() == 8 && LineReader<64>::capacity() == 64);

    std::println("\n{}", failures ? "SOME TESTS FAILED" : "all passed");
    return failures != 0;
}
