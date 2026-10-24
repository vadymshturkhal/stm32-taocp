// test_wait_list_stats.cpp - the optional WAIT list counters (KNUTH_KERNEL_STATS), run on the PC
//
//   length:  immed and sortin add a node, delete / delete_first remove one, a
//            re-hold of a waiting node moves it (the length stays), cancelling a
//            node that is not held changes nothing
//   sortins: hold and holdc count, immed does not
//   walk:    nodes passed from the tail: 0 for the latest time, all of them for
//            the earliest, equal times go after the ones already there (FIFO)
//   off:     without the macro the counters stay zero (checked in test_wait_list)
//
// Build (from tests/):  g++-14 -std=c++23 -I.. -O2 -Wall -Wextra -Wshadow -Wconversion -Wsign-conversion
//             test_wait_list_stats.cpp -o test_wait_list_stats
// Exit status: 0 if every check passed.
#define KNUTH_KERNEL_STATS
#include <cstdint>
#include <print>
#include <string>

#include "wait_list.hpp"

static int failures = 0;

static void check(bool ok, const std::string& what) {
    if (!ok) {
        ++failures;
        std::println("FAIL: {}", what);
    }
}

static void noop(WaitNode&, std::uint32_t) noexcept {}

static void test_length() {
    WaitList list;
    WaitNode a{noop, 0}, b{noop, 0}, c{noop, 0};

    check(list.length() == 0, "length: empty");
    list.hold(a, 0, 10);
    list.hold(b, 0, 20);
    list.immed(c, 0);
    check(list.length() == 3, "length: two holds and an immed");

    list.hold(a, 0, 30);                       // a waits already: it moves
    check(list.length() == 3, "length: re-hold keeps it");

    list.delete_node(b);
    check(list.length() == 2, "length: delete");
    list.delete_node(b);                       // not held any more
    check(list.length() == 2, "length: delete twice changes nothing");

    (void)list.delete_first();
    (void)list.delete_first();
    check(list.length() == 0, "length: delete_first to empty");
    check(list.delete_first() == nullptr && list.length() == 0, "length: delete_first on empty");
}

static void test_sortins_and_walk() {
    WaitList list;
    WaitNode n[4] = {{noop, 0}, {noop, 0}, {noop, 0}, {noop, 0}};

    list.hold(n[0], 0, 10);                    // empty list: length 0, walk 0
    list.hold(n[1], 0, 20);                    // the latest: length 1, walk 0
    list.hold(n[2], 0, 5);                     // the earliest: length 2, walk 2
    list.holdc(n[3], 0, 10, noop);             // equal to n[0]: after it, length 3, walk 1 (passes 20)

    const WaitListStats& s = list.stats();
    check(s.sortins == 4, "sortins: hold and holdc");
    check(s.length_sum == 0 + 1 + 2 + 3, "length_sum");
    check(s.length_max == 3, "length_max");
    check(s.walk_sum == 0 + 0 + 2 + 1, "walk_sum");
    check(s.walk_max == 2, "walk_max");

    WaitNode m{noop, 0};
    list.immed(m, 0);
    check(list.stats().sortins == 4, "sortins: immed does not sort");
    check(list.length() == 5, "length: immed counts");

    // FIFO for equal times: n[0] (10) runs before n[3] (10)
    (void)list.delete_first();                 // m
    (void)list.delete_first();                 // n[2], at 5
    check(list.delete_first() == &n[0], "equal times: the first one held first");
    check(list.delete_first() == &n[3], "equal times: then the second");
}

int main() {
    static_assert(WAIT_LIST_STATS, "this test needs KNUTH_KERNEL_STATS");
    test_length();
    test_sortins_and_walk();

    if (failures == 0) std::println("test_wait_list_stats: all checks passed");
    return failures == 0 ? 0 : 1;
}
