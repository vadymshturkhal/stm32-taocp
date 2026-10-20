# Elevator on the Knuth kernel (C++)

Knuth's elevator simulation (TAOCP Vol. 1, §2.2.5) ported to C++ for the STM32
Nucleo-G431RB, running on the [Knuth kernel](../../knuth_kernel).

**What makes this port different:** the 1968 discrete-event design runs,
unchanged in spirit, as the scheduler of a real microcontroller. Knuth's WAIT
list, `HOLD`, `IMMED` and `CYCLE` are the kernel itself; the elevator's steps
E1–E9 and the users' steps U1–U6 are nodes in it. The trace it prints can be
checked against the book, and against the [C version](../C_Embedded) of the
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
  the node's back-pointer and sets `TIME` from the node's `next_time()`, as C's
  `cycle()` did with `TIME = C->NEXTTIME`. `now` (the real clock) is not used:
  the elevator runs in Knuth's time units.
- **Helpers** (`E1A`, `E2A`, `E4A`, `E5A`, `E6B`) are ordinary member functions,
  called from inside a step. They use the `simulation` member and never call
  `begin_step`: `TIME` was already set by the step that called them.
- **Steps are short** (the kernel's first rule). `E4` is the example: it finds
  one user to let out or in, starts that user's node (`immed`), holds itself
  for 25 units and returns. It could handle every user on the floor in one
  loop, but a long step blocks the main loop and every other program on the
  kernel. One user per step is also Knuth's model: each user takes 25 units.
- A step that jumps to another step (`E2` → `E3`) calls it directly with the
  same node and `now`, as C does; it is not scheduled.

**Status:** in progress. Done: the shared state (`ElevatorSimulation`), the node
(`ElevatorNode`, a `WaitNode` with the queue links and the user's fields) and
the queues (`ElevatorList`). Next: the elevator's and the users' steps, then a
PC run whose trace must match the C version's byte for byte, then the board.

Build (from this folder), on the PC:

```
g++-14 -std=c++23 -I. -I../../knuth_kernel -Wall -Wextra -Wconversion main.cpp elevator_list.cpp -o main && ./main
```
