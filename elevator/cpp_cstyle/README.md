# Elevator: the C design in C++

[![Elevator (C-style C++)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-cpp-cstyle.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-cpp-cstyle.yml)

The [C elevator](../c_embedded) (TAOCP Vol. 1, §2.2.5) ported **1:1** to C++,
to answer one question: **is C++ itself slower than C?**

Everything that decides speed is kept exactly as in C:

- one `malloc`'d block for the storage pool, the shared state, the elevator and
  the users;
- a `SharedState*` passed to every function (no back-pointers);
- the WAIT list with Knuth's sentinel (the head's `NEXTTIME` is 0) and
  **unsigned** compares (simulated time that never wraps);
- the same functions in the same files, so the same calls between files
  (`elevator.cpp` calls `hold` in `elevator_settings.cpp`, as `elevator.c` does).

Only spellings that cost nothing differ: `namespace cstyle`, `constexpr`
instead of `#define`, `nullptr`, `static_cast`, `bool`.

It is the middle one of three versions compared on the STM32G4 board
([RESULTS.md](../measurements/RESULTS.md#embedded-c-vs-c-stm32g4-cortex-m4--dwt-cyccnt-66-users)):

| `-O3` on all three | [C](../c_embedded) | this (C-style C++) | [C++ on the Knuth kernel](../cpp_embedded_kernel) |
|---|---|---|---|
| no LTO | 198 cycles/step | **198** | 197 |
| `-flto` | 142 | **140** | 171 |

The same design gives the same cycles in both languages: C++ costs nothing
here. The kernel version's extra ~20% with LTO is the price of its guarantees
(wrap-safe 32-bit time, no globals, re-hold = restart), not of C++.

## Layout

| Path | What it is |
|------|------------|
| `cstyle.hpp` | the structs and declarations (C's headers in one file) |
| `elevator.cpp`, `users.cpp`, `elevator_settings.cpp`, `elevator_list.cpp`, `storage_pool.cpp`, `values.cpp`, `trace.cpp` | C's `.c` files, one for one |
| `cstyle_loop.{h,cpp}` | the run split into start / one step / stop (C-callable, used by the board's `main.c`) |
| `pc/main.cpp` | the PC entry point |

## Build, run, compare (on the PC)

From this folder (`elevator/cpp_cstyle`):

```
g++-14 -std=c++23 -I. -O2 -Wall -Wextra -Wshadow -Wconversion -Wsign-conversion -DTRACE *.cpp pc/main.cpp -o elevator_sim
./elevator_sim 4
for n in 0 1 4 26 500; do ./elevator_sim $n | diff -q - ../measurements/trace_$n.txt && echo "trace_$n OK"; done
```

`-DTRACE` turns the trace on; without it `trace()` is an empty inline function.
CI builds it with `-Werror`, diffs all five golden traces, and repeats them under
AddressSanitizer and UndefinedBehaviorSanitizer.

---

*Code, design and measurements are mine; Claude helped draft the READMEs and the paper, and reviewed code.*
