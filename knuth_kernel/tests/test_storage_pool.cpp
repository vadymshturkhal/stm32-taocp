// test_storage_pool.cpp - tests of StoragePool<T> (storage_pool.hpp), run on the PC
//
//   constructor: count 0 or a count whose size overflows gives a pool that is
//                not valid() and gives nothing
//   pop / push:  exactly count nodes, then nullptr (overflow); the nodes are one
//                block: distinct, aligned, not overlapping; push then pop gives the
//                same node back (LIFO, like C's AVAIL); owns()
//   the link:    a free node keeps the AVAIL link in WaitNode::left; a popped node
//                points to itself again (is_held false, cancel on it changes nothing)
//   lifetime:    every node is constructed once by the constructor and destroyed once with
//                the pool; a reused node keeps its previous user's fields (the
//                caller sets them)
//   with a list: a popped node held and run by a WaitList, pushed back, reused
//   random:      100 000 random pops and pushes compared with a model (the set of
//                nodes in use): never a node twice, overflow exactly at count
//   double push: is_free() for a popped, a held, a pushed node; pushing a free node
//                again (the AVAIL head or another one), a held node, or a node from
//                another pool aborts with the asserts on (checked in a child process)
//   bad states:  the valid paths (held, cancelled twice, pushed, popped; a running
//                step that gives its own node back, like the elevator's U6); cancel,
//                hold, holdc, immed or Kernel::cancel on a free node abort with the
//                asserts on, and with NDEBUG (the board's build) every misuse is
//                refused: the list stays empty, the pool gives each node once
//   refused:     the counters stay 0 on the valid paths; with NDEBUG each refused
//                operation counts once (list, Kernel, pool), and so does a due node
//                whose step was cleared while it waited (skipped, not run)
//
// Build (from tests/):  g++-14 -std=c++23 -I.. -O2 -Wall -Wextra -Wshadow -Wconversion -Wsign-conversion
//             test_storage_pool.cpp -o test_storage_pool
// Exit status: 0 if every check passed.
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <print>
#include <random>
#include <csignal>
#include <cstdio>
#include <set>
#include <vector>

#include <sys/wait.h>
#include <unistd.h>

#include "kernel.hpp"
#include "storage_pool.hpp"
#include "wait_list.hpp"

static int failures = 0;

static void check(const char* name, bool ok) {
    std::println("{}  {}", ok ? "PASS" : "FAIL", name);
    failures += !ok;
}

// A node like Knuth's user node: a WaitNode with its own data.
struct UserNode : WaitNode {
    std::uint32_t IN = 0;
    std::uint32_t OUT = 0;
    std::uint32_t runs = 0;
};

static void count_run(WaitNode& n, std::uint32_t) noexcept { ++static_cast<UserNode&>(n).runs; }

// A node that counts its constructions and destructions.
static int constructed = 0;
static int destroyed = 0;
struct Counted : WaitNode {
    Counted() noexcept { ++constructed; }
    ~Counted() { ++destroyed; }
};

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

// Pops until nullptr: true if that gives exactly n different nodes (AVAIL intact).
template <class T>
static bool pops_each_once(StoragePool<T>& pool, std::size_t n) {
    std::set<T*> seen;
    for (std::size_t i = 0; i <= n; ++i) {
        T* p = pool.pop();
        if (p == nullptr) return i == n;
        if (!seen.insert(p).second) return false;   // a node twice: AVAIL is corrupt
    }
    return false;                                   // more nodes than the pool holds
}

// A step that gives its own node back while it runs, like the elevator's U6
static StoragePool<UserNode>* g_pool = nullptr;
static WaitList* g_list = nullptr;
static void give_back(WaitNode& n, std::uint32_t) noexcept {
    g_list->delete_node(n);                 // running: in no list, so this changes nothing
    g_pool->push(static_cast<UserNode&>(n));
}

static bool aligned(const void* p, std::size_t a) {
    return reinterpret_cast<std::uintptr_t>(p) % a == 0;
}

int main() {
    // 1. The constructor
    {
        StoragePool<UserNode> zero(0);
        check("count 0: not valid, pop gives nullptr", !zero.valid() && zero.pop() == nullptr);

        StoragePool<UserNode> huge(SIZE_MAX / sizeof(UserNode) + 1);
        check("a count whose size overflows size_t: not valid", !huge.valid() && huge.pop() == nullptr);

        StoragePool<UserNode> four(4);
        check("count 4: valid", four.valid());
    }

    // 2. Exactly count nodes, one block, distinct, aligned, not overlapping
    {
        constexpr std::size_t N = 16;
        StoragePool<UserNode> pool(N);
        const bool created = pool.valid();
        std::vector<UserNode*> nodes;
        for (std::size_t i = 0; i < N; ++i) nodes.push_back(pool.pop());
        const bool all = std::ranges::none_of(nodes, [](UserNode* p) { return p == nullptr; });
        check("16 pops from a pool of 16 all succeed, the 17th is nullptr",
              created && all && pool.pop() == nullptr);

        std::vector<UserNode*> sorted = nodes;
        std::ranges::sort(sorted);
        bool apart = true;
        for (std::size_t i = 1; i < sorted.size(); ++i) apart = apart && sorted[i] == sorted[i - 1] + 1;
        check("the 16 nodes are one block: distinct, one node apart, no overlap", apart);

        check("every node is aligned for its type",
              std::ranges::all_of(nodes, [](UserNode* p) { return aligned(p, alignof(UserNode)); }));

        UserNode outside;
        check("owns: true for its own nodes, false for another node",
              std::ranges::all_of(nodes, [&](UserNode* p) { return pool.owns(*p); }) && !pool.owns(outside));

        for (UserNode* p : nodes) pool.push(*p);
    }

    // 3. LIFO, like C's AVAIL
    {
        StoragePool<UserNode> pool(4);
        UserNode* a = pool.pop();
        UserNode* b = pool.pop();
        pool.push(*a);
        pool.push(*b);
        UserNode* c = pool.pop();
        UserNode* d = pool.pop();
        check("push a, push b, then pop gives b, then a (LIFO)", c == b && d == a);
    }

    // 4. The link lives in WaitNode::left only while the node is free
    {
        StoragePool<UserNode> pool(3);
        WaitList wait;
        UserNode* a = pool.pop();
        UserNode* b = pool.pop();
        check("a free node is not held (right points to itself)", !b->is_held());

        wait.delete_node(*a);                       // cancel a popped node: left must point to itself
        UserNode* c = pool.pop();
        check("cancel on a popped node changes nothing: the pool still gives its last node, then nullptr",
              c != nullptr && c != a && c != b && pool.pop() == nullptr);
        pool.push(*a);
        pool.push(*b);
        pool.push(*c);
    }

    // 5. Lifetime: constructed once by the constructor, destroyed once with the pool; fields are kept
    {
        constructed = destroyed = 0;
        {
            StoragePool<Counted> pool(5);
            const int after_ctor = constructed;
            for (int i = 0; i < 100; ++i) {
                Counted* n = pool.pop();
                pool.push(*n);
            }
            check("the constructor builds every node once; pop and push construct nothing",
                  after_ctor == 5 && constructed == 5 && destroyed == 0);
        }
        check("the pool destroys every node once", destroyed == 5);

        StoragePool<UserNode> pool(1);
        UserNode* u = pool.pop();
        u->IN = 3;
        u->OUT = 1;
        u->set_next_inst(&count_run);
        pool.push(*u);
        UserNode* v = pool.pop();
        check("a reused node keeps its previous user's fields (the caller must set them)",
              v == u && v->IN == 3 && v->OUT == 1 && v->get_next_inst() == &count_run && !v->is_held());
        pool.push(*v);
    }

    // 6. A popped node in a WaitList: held, run, pushed back, reused
    {
        StoragePool<UserNode> pool(2);
        WaitList wait;
        UserNode* u = pool.pop();
        u->set_next_inst(&count_run);
        wait.hold(*u, 0, 10);
        const bool held = u->is_held();
        if (WaitNode* due = wait.is_node_ready(10) ? wait.delete_first() : nullptr) due->run(10);
        const bool ran = u->runs == 1 && !u->is_held();
        pool.push(*u);
        UserNode* again = pool.pop();
        again->runs = 0;
        wait.hold(*again, 10, 5);
        if (WaitNode* due = wait.is_node_ready(15) ? wait.delete_first() : nullptr) due->run(15);
        check("pop -> hold -> run -> push -> pop -> hold -> run, the list empty after",
              held && ran && again == u && again->runs == 1 && wait.is_empty());
        pool.push(*again);
    }

    // 7. Random pops and pushes against a model
    {
        constexpr std::size_t N = 16;
        StoragePool<UserNode> pool(N);
        std::set<UserNode*> in_use;                     // the model
        std::vector<UserNode*> order;                   // the same nodes, to push a random one
        std::mt19937 rng(12345);
        bool ok = true;
        std::size_t overflows = 0;
        for (int step = 0; step < 100'000 && ok; ++step) {
            if (rng() % 2 == 0) {
                UserNode* p = pool.pop();
                if (in_use.size() == N) {
                    ok = p == nullptr;                  // overflow exactly when all are in use
                    ++overflows;
                } else {
                    ok = p != nullptr && !in_use.contains(p) && pool.owns(*p) && !p->is_held();
                    if (ok) { in_use.insert(p); order.push_back(p); }
                }
            } else if (!order.empty()) {
                const std::size_t i = rng() % order.size();
                UserNode* p = order[i];
                order[i] = order.back();
                order.pop_back();
                in_use.erase(p);
                pool.push(*p);
            }
        }
        check("100 000 random pops / pushes: never the same node twice, nullptr exactly when all 16 are in use",
              ok && overflows > 0);
        for (UserNode* p : order) pool.push(*p);
    }

    // 8. A node pushed twice: caught before it can corrupt AVAIL
    {
        StoragePool<UserNode> pool(4);
        UserNode* a = pool.pop();
        check("a popped node is not free", !pool.is_free(*a));
        WaitList list;
        a->set_next_inst(&count_run);
        list.hold(*a, 0, 10);
        check("a held node is not free", !pool.is_free(*a));
        list.delete_node(*a);
        check("out of the list again: still not free", !pool.is_free(*a));
        pool.push(*a);
        check("a pushed node is free", pool.is_free(*a));
        UserNode* b = pool.pop();
        check("and pop gives it back", b == a && !pool.is_free(*a));
    }
#ifndef NDEBUG
    check("pushing the AVAIL head again aborts", aborts([] {
        StoragePool<UserNode> pool(4);
        UserNode* a = pool.pop();
        pool.push(*a);                      // a is the AVAIL head now
        pool.push(*a);                      // would link a to itself
    }));
    check("pushing a free node that is not the head aborts", aborts([] {
        StoragePool<UserNode> pool(4);
        UserNode* a = pool.pop();
        UserNode* b = pool.pop();
        pool.push(*a);
        pool.push(*b);                      // head is b, a is below it
        pool.push(*a);
    }));
    check("pushing a node that is still held aborts", aborts([] {
        StoragePool<UserNode> pool(4);
        WaitList list;
        UserNode* a = pool.pop();
        a->set_next_inst(&count_run);
        list.hold(*a, 0, 10);
        pool.push(*a);
    }));
    check("pushing a node from another pool aborts", aborts([] {
        StoragePool<UserNode> pool(4);
        StoragePool<UserNode> other(4);
        pool.push(*other.pop());
    }));
#endif

    // 9. Nodes in bad states: the valid paths first
    {
        StoragePool<UserNode> pool(2);
        WaitList list;
        UserNode* a = pool.pop();
        a->set_next_inst(&count_run);
        list.hold(*a, 0, 10);
        list.delete_node(*a);
        list.delete_node(*a);                       // cancel twice: the second changes nothing
        pool.push(*a);
        check("held, cancelled twice, pushed: free, the list empty, the pool whole",
              pool.is_free(*a) && list.is_empty() && pops_each_once(pool, 2));
        check("the valid path refuses nothing", list.refused() == 0 && pool.refused() == 0);
    }
    {
        StoragePool<UserNode> pool(2);
        WaitList list;
        g_pool = &pool;
        g_list = &list;
        UserNode* a = pool.pop();
        a->set_next_inst(&give_back);
        list.hold(*a, 0, 10);
        WaitNode* due = list.delete_first();        // the kernel's step: out of the list, then run
        due->run(10);
        check("a running step cancels and pushes its own node (U6): free, the pool whole",
              due == a && pool.is_free(*a) && list.is_empty() && pops_each_once(pool, 2));
        check("the U6 path refuses nothing", list.refused() == 0 && pool.refused() == 0);
    }

#ifndef NDEBUG
    // With the asserts on, every misuse of a free node stops at once
    check("cancel of a free node aborts", aborts([] {
        StoragePool<UserNode> pool(4);
        WaitList list;
        UserNode* a = pool.pop();
        pool.push(*a);
        list.delete_node(*a);
    }));
    check("hold of a free node aborts", aborts([] {
        StoragePool<UserNode> pool(4);
        WaitList list;
        UserNode* a = pool.pop();
        a->set_next_inst(&count_run);
        pool.push(*a);
        list.hold(*a, 0, 10);
    }));
    check("holdc of a free node aborts", aborts([] {
        StoragePool<UserNode> pool(4);
        WaitList list;
        UserNode* a = pool.pop();
        pool.push(*a);
        list.holdc(*a, 0, 10, &count_run);
    }));
    check("immed of a free node aborts", aborts([] {
        StoragePool<UserNode> pool(4);
        WaitList list;
        UserNode* a = pool.pop();
        a->set_next_inst(&count_run);
        pool.push(*a);
        list.immed(*a, 0);
    }));
    check("Kernel::cancel of a free node aborts", aborts([] {
        StoragePool<UserNode> pool(4);
        Kernel k;
        UserNode* a = pool.pop();
        pool.push(*a);
        k.cancel(*a);
    }));
#else
    // With NDEBUG (the board's build) every misuse is refused and changes nothing
    {
        StoragePool<UserNode> pool(4);
        WaitList list;
        UserNode* a = pool.pop();
        UserNode* b = pool.pop();
        UserNode* c = pool.pop();
        b->set_next_inst(&count_run);
        pool.push(*a);
        pool.push(*b);
        pool.push(*c);                              // AVAIL: c, b, a, and the 4th node
        list.delete_node(*b);                       // b is free and in the middle of AVAIL
        list.hold(*b, 0, 10);
        list.holdc(*b, 0, 10, &count_run);
        list.immed(*b, 0);
        Kernel k;
        k.cancel(*b);
        pool.push(*b);                              // a second push
        check("NDEBUG: cancel, hold, holdc, immed, Kernel::cancel and a second push of a free node are refused",
              list.is_empty() && pool.is_free(*a) && pool.is_free(*b) && pool.is_free(*c) && !a->is_held()
              && pops_each_once(pool, 4));
        check("NDEBUG: each refusal counted once (list 4, Kernel 1, pool 1)",
              list.refused() == 4 && k.refused() == 1 && pool.refused() == 1);
    }
    {
        StoragePool<UserNode> pool(2);
        WaitList list;
        UserNode* a = pool.pop();
        a->set_next_inst(&count_run);
        a->runs = 0;
        list.hold(*a, 0, 10);
        pool.push(*a);                              // still held: refused
        WaitNode* due = list.delete_first();
        if (due != nullptr) due->run(10);
        check("NDEBUG: a push of a held node is refused, the node still runs",
              due == a && a->runs == 1 && !pool.is_free(*a));
        check("NDEBUG: and counted", pool.refused() == 1);
    }
    {
        Kernel k;
        UserNode a;
        a.set_next_inst(&count_run);
        a.runs = 0;
        k.hold(a, 0, 10);
        a.set_next_inst(nullptr);                   // cleared while it waits
        const bool stepped = k.step(10);
        check("NDEBUG: a due node with no step is skipped, and counted",
              stepped && a.runs == 0 && k.refused() == 1 && !k.get_next_time());
    }
#endif

    std::println("\n{}", failures ? "SOME TESTS FAILED" : "all passed");
    return failures != 0;
}
