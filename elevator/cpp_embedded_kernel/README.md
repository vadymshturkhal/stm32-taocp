# Elevator on the Knuth kernel (C++)

[![Elevator (C++)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-cpp.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-cpp.yml)

Knuth's elevator simulation (TAOCP Vol. 1, §2.2.5) ported to C++ for the STM32
Nucleo-G431RB, running on the [Knuth kernel](../../knuth_kernel).

**What makes this port different:** the 1968 discrete-event design runs,
unchanged in spirit, as the scheduler of a real microcontroller. Knuth's WAIT
list, `HOLD`, `IMMED` and `CYCLE` are the kernel itself; the elevator's steps
E1–E9 and the users' steps U1–U6 are nodes in it. The trace it prints can be
checked against the book, and against the [C version](../c_embedded) of the
same program.

## Steps and helpers

- **Steps** (E1–E9, U1–U6) are `static` and have the signature
  `void (WaitNode& node, std::uint32_t now) noexcept`. It is not the
  elevator's choice: it is the kernel's `WaitNode::NextInst`
  ([wait_list.hpp](../../knuth_kernel/wait_list.hpp)), the type of a node's
  `NEXTINST`. The kernel runs every node the same way, `NEXTINST(node, now)`,
  without knowing which program it belongs to, so every step of every program
  has this signature.
  Each one begins with `begin_step(node)`, which finds the simulation through
  the node's back-pointer and sets `TIME` from the node's `get_next_time()`, as C's
  `cycle()` did with `TIME = C->NEXTTIME`. `now` (the real clock) is not used:
  the elevator runs in Knuth's time units.
- **Helpers** (`E1A`, `E2A`, `E4A`, `E5A`, `E6B`, `U4A`, `U6_impl`, and `U3`) are ordinary member functions,
  called from inside a step. They use the `simulation` member and never call
  `begin_step`: `TIME` was already set by the step that called them.
- **Steps are short** (the kernel's first rule). `E4` is the example: it finds
  one user to let out or in, starts that user's node (`immed`), holds itself
  for 25 units and returns. It could handle every user on the floor in one
  loop, but a long step blocks the main loop and every other program on the
  kernel. One user per step is also Knuth's model: each user takes 25 units.
- **Cancelling is the kernel's job, not the steps'.** C copies a MIX trick:
  E3 checks `ELEV3->left1 != NULL` to know whether E9 is still scheduled and
  then cancels it (`DELETEW`), and E9 (and the init) zero that cell. On the
  kernel no step needs this code, because the kernel keeps two invariants:
  - **a running node is never in the list:** `step` takes the node out before
    running it, so when `E9` runs on `ELEV3`, `ELEV3` is already unlinked;
  - **`hold` on a waiting node restarts it:** when `E3` holds `ELEV3` for 300,
    the kernel unlinks it first if it is still waiting for E9, so Knuth's
    `DELETEW` + `HOLD` is one call.

  (`cancel` on a node that is not in the list does nothing, so an explicit
  `cancel` would be harmless, only redundant.) A real cancel is still needed
  where a waiting node must not run and nothing re-holds it: `E6B` cancels
  E9 (`ELEV3`) when the elevator leaves with `D2` set; only C's `STZ` mark
  goes away there.
- A step that jumps to another step (`E2` → `E3`) calls it directly with the
  same node and `now`, as C does; it is not scheduled.
- **Only a scheduled step needs the kernel's signature.** Knuth's U3 (enter
  the queue) is only ever jumped to from U2, never a node's `NEXTINST`, so it
  is a helper: `U3(ElevatorNode& user)`. E3, E7 and E8 are jumped to as well,
  but they are also scheduled (by `DECISION`, U2 and `E6B`), so they stay steps.
- **U2 must be a helper, not a step.** U1 jumps to U2 with the *new* user's
  node, which has never been scheduled: its `get_next_time()` is 0 or a
  previous user's old time. A `begin_step(user)` there would set `TIME` from
  that stale value and every `hold`/`immed` after it would use the wrong time.
  As a helper, `U2(ElevatorNode& user)` keeps the `TIME` that U1's
  `begin_step(USER1)` set.
- **`DECISION`'s caller from U2 is `&U1`.** `decision(caller)` only checks
  `caller == E6`; every other caller behaves the same. U2 is not a step any
  more, so it passes the step that is actually running, U1:
  `simulation.decision(&U1);  // Knuth: DECISION from U2 (U2 runs inside U1's step)`.

**Status:** runs on the PC, and its trace matches the C version byte for byte:
`trace_0`, `trace_1`, `trace_4`, `trace_26` and `trace_500` in
[`../measurements`](../measurements) (also under AddressSanitizer and
UndefinedBehaviorSanitizer, with the asserts on). CI
([elevator-cpp.yml](../../.github/workflows/elevator-cpp.yml)) checks all five on
every push that changes this folder, the kernel or the traces.

**On the board:** it runs in real time on the STM32G431, the kernel in PendSV
woken by the TIM2 compare: the same 544 steps and checksum as on the PC, every
step within 6 µs of its deadline (301 ns on average), the CPU asleep 99.8 % of
the time. See [`board/`](board).

## Layout

| Path | What it is |
|------|------------|
| `*.cpp`, `*.hpp` (this folder) | the elevator itself: steps, users, lists, `VALUES`, trace; **no `main`**, the same sources for the PC and the board |
| `pc/main.cpp` | the PC entry point: users count from `argv`, the trace header, the kernel run event-driven until the WAIT list is empty |
| `board/` | the G431 applications, one folder per variant (now [`pendsv/`](board/pendsv): the kernel in PendSV; later a tickless one): each with its own `main` (the link, sleep, the measurements) and the C-callable start/stop, running the same elevator in real time ([README](board/README.md)) |

The PC build names its entry point explicitly (`*.cpp pc/main.cpp`), so a
board `main` can never end up in the PC build, or the other way round. The
trace ends lines with `\n` like the golden traces; on the board the UART
output (`_write`) will turn `\n` into `\r\n`, so the core stays the same.

## Build, run, compare (on the PC)

From this folder (`elevator/cpp_embedded_kernel`):

**Build** (with the trace on):

```
g++-14 -std=c++23 -I. -I../../knuth_kernel -O2 -Wall -Wextra -Wshadow -Wconversion -Wsign-conversion -DTRACE *.cpp pc/main.cpp -o elevator_sim
```

`-DTRACE` turns the trace on; without it `trace()` is an empty inline function
and the program runs silently.

**Run** with N users (4 if no argument):

```
./elevator_sim 4
./elevator_sim 26 | less
```

```
TIME STATE FLOOR D1 D2 D3 step action
0000   N     2    0  0  0   U1 User 0 arrives at floor 4, destination is 1, give up 370
0035   U     2    0  0  0   E7 Elevator moving up
0086   U     3    0  0  0   E7 Elevator moving up
0135   U     4    0  0  0   U1 User 1 arrives at floor 4, destination is 2, give up 250
0151   U     4    0  0  0   E2 Elevator stops
...
```

**Compare** with the golden traces (made by the C version):

```
for n in 0 1 4 26 500; do ./elevator_sim $n | diff -q - ../measurements/trace_$n.txt && echo "trace_$n OK"; done
```

Every line must say `OK`; `./elevator_sim N | diff - ../measurements/trace_N.txt`
shows the first difference otherwise. To run the same check under the
sanitizers (with the asserts on), build with
`-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all` instead of `-O2`.

`main` runs the kernel event-driven, like Knuth's `CYCLE`: it jumps to
`get_next_time()` and runs every due node, until the WAIT list is empty.

---

*Code, design and measurements are mine; Claude helped draft the READMEs and the paper, and reviewed code.*
