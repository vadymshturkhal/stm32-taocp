# The Art of Computer Programming on STM32: C, C++, Python and ARM Assembly

[![Elevator (Python)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-python.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-python.yml)
[![Elevator (C)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-c.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-c.yml)
[![Elevator (C++ on the Knuth kernel)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-cpp.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-cpp.yml)
[![Elevator (C-style C++)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-cpp-cstyle.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-cpp-cstyle.yml)
[![Knuth kernel](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/knuth-kernel.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/knuth-kernel.yml)
[![Pi-MCU protocol](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/pi-mcu-protocol.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/pi-mcu-protocol.yml)

- This repository contains Python, C, C++ and hand-tuned ARM Assembly implementations of [Algorithms](#algorithms) and [Data Structures](#data-structures) from The Art of Computer Programming, Vol.1, and a [Raspberry Pi ↔ STM32 protocol](#systems) built from its ideas.
- [Knuth kernel](#systems): Knuth's 1968 CYCLE running as a real-time kernel on the Cortex-M4, at two interrupt priority levels woken by a hardware timer: every event within 6 µs of its deadline, the CPU asleep 99.8% of the time, the same result as the simulation to the last bit.
- [Firmware](#firmware): the elevator on the Knuth kernel from one script: it builds, flashes the board and checks the run, with an MPU guard against stack overflow.
- [Paper](#papers): *Is C++ Slower than C on a Cortex-M4?* A controlled measurement on the board: the same design costs the same in both languages.

## Why this repository exists

I planned to go through TAOCP in Python, until I saw that Knuth writes his programs in the MIX language and uses cycle counting methodology. I had two choices: to give up or to go further. Unexpectedly, I went further and bought a NUCLEO-G431RB board to test TAOCP's concepts on real hardware. This repository holds my results and my answer to the question: *can the concepts from The Art of Computer Programming be used on real hardware?*

## Hardware and tools

| What | Used |
|------|------|
| Board | NUCLEO-G431RB: STM32G431RB, Arm Cortex-M4 with FPU, 170 MHz, 128 KB flash, 32 KB RAM; the on-board ST-LINK for flashing, debugging and a USB serial port |
| MCU toolchain, CubeIDE | STM32CubeIDE: `arm-none-eabi-gcc` / `g++` 14.3 (GNU Tools for STM32), the HAL; the kernel and the elevator [linked from this checkout](elevator/cpp_embedded_kernel/board/CUBEIDE.md) |
| MCU toolchain, without CubeIDE | [firmware/kernel_elevator](firmware/kernel_elevator): CMake ≥ 3.20, `arm-none-eabi-gcc` ≥ 10 (Ubuntu 22.04: 10.3), no HAL |
| Flash and debug | OpenOCD and GDB (`arm-none-eabi-gdb` or `gdb-multiarch`), or CubeIDE's debugger |
| Measuring on the board | the DWT cycle counter; the trace over LPUART1 DMA, read at 921600 baud on the ST-LINK serial port |
| PC builds and tests | `gcc`, `g++-14` (C++23), Python 3.10+ |
| Host for the protocol | Raspberry Pi 4 (a Linux PC over the ST-LINK USB serial port during development) |
| CI | GitHub Actions on Ubuntu 24.04: the badges above; each workflow builds and tests one project on every push; the C++ ones with `-Werror` and ASan + UBSan, the kernel and the protocol also TSan |

# Algorithms:
- [Algorithm P (Print table of 500 primes)](knuth_primes)
- [Algorithm A (Multiply permutations in cycle form)](multiply_permutations_a)
- [Algorithm B (Multiply permutations in cycle form)](multiply_permutations_b)
- [Algorithm I (Inverse permutation in place)](inverse_permutation_in_place)
- [Algorithm J (Inverse permutation in place)](inverse_permutation_in_place_j)
- [Algorithm T (Topological Sort)](topological_sort)
- [Algorithm A (Addition of polynomials)](polynomials/addition_of_polynomials)
- [Algorithm M (Multiplication of polynomials)](polynomials/multiplication_of_polynomials)
- [Elevator: discrete simulation program](elevator) ([Python](elevator/python_wait_list), [C](elevator/c_embedded), [C (native)](elevator/c_native), [ARM Assembly](elevator/assembly), [Python (async)](elevator/python_async), [C++ on the Knuth kernel](elevator/cpp_embedded_kernel), [C-style C++](elevator/cpp_cstyle), [Measurements](elevator/measurements))
  <details>
  <summary>Knuth's MIX elevator simulation (TAOCP Vol. 1, 2.2.5), implemented across languages and targets</summary>

  - [Python](elevator/python_wait_list): subroutine-based port driven by an explicit WAIT list, without `yield`/`async`
  - [C](elevator/c_embedded): embedded-target port for the STM32G4, paired with the ARM Assembly port below
  - [C (native)](elevator/c_native): standalone laptop build (malloc-based, no STM32 dependencies), used for timing comparisons against Python
  - [ARM Assembly](elevator/assembly): hand-tuned Cortex-M4 Thumb-2 port, paired with the embedded C port above
  - [Python (async)](elevator/python_async): coroutine-based port using `asyncio`/`await` instead of an explicit WAIT list
  - [C++ on the Knuth kernel](elevator/cpp_embedded_kernel) *(runs on the PC and on the STM32G4)*: the 1968 discrete-event design running, unchanged in spirit, as the scheduler of a real microcontroller: the steps E1–E9 and U1–U6 are nodes of the [Knuth kernel](knuth_kernel)'s WAIT list, users come from its storage pool, and the trace matches the C port byte for byte
  - [C-style C++](elevator/cpp_cstyle): the C port's design rewritten 1:1 in C++ (same structs, same calls, same unsigned compares), to measure the language alone: on the STM32G4 it runs at the same cycles per step as C
  - [Measurements](elevator/measurements): the golden traces the ports are checked against (in CI for C, both C++ ports and Python), and scripts comparing timing and trace-output correctness between the C (native) and Python ports -- see [RESULTS.md](elevator/measurements/RESULTS.md) for measured numbers (ASM vs GCC and C vs C++ on STM32, C vs Python on laptop)
  </details>

  **Measured** ([RESULTS.md](elevator/measurements/RESULTS.md)): the same 66-user simulation on the NUCLEO-G431RB at 170 MHz, DWT cycle counts:

  | Elevator on the STM32G4 | Cycles | |
  |---|---|---|
  | C, `-O3` | 90 322 (whole run) | |
  | C, `-O3 -flto` | 77 794 | |
  | Hand-tuned ARM Assembly | **70 140** | 10% faster than the compiler's best |

  C vs C++, cycles per step, one node per main-loop pass beside the Pi link (a checksum proves all three run the same 544 steps in the same order):

  | Per step | `-O3` | `-O3 -flto` |
  |---|---|---|
  | Hand-tuned ARM Assembly | **~129**\* | **~129**\* |
  | C | 198 | 142 |
  | C's design in C++ | 198 | 140 |
  | C++ on the Knuth kernel | 197 | 171 |

  \* The whole run (70 140 cycles, setup included) ÷ 544 steps: measured as one batch, not per pass. The same calculation for C with LTO gives 143 against the 142 measured per step. The compiler flags only touch the assembly's C glue.

  Flash used by the elevator, in bytes (Cortex-M4 builds of each version with a minimal `main`, `.text` minus an empty program that only calls `malloc`/`free`: steps, scheduler, lists, storage pool, `VALUES` and start/stop glue):

  | Flash | `-O3` | `-O3 -flto` | LTO |
  |---|---|---|---|
  | Hand-tuned ARM Assembly | **2 116** | **2 052** | −64 (its C glue only) |
  | C | 2 688 | 3 292 | +604 (+22%) |
  | C's design in C++ | 2 824 | 3 360 | +536 (+19%) |
  | C++ on the Knuth kernel | 5 144 | 5 304 | +160 (+3%) |

  The same design costs the same in C and C++, in time and in flash: C++ itself is free. LTO buys C its speed with flash: it removes the calls between files by inlining `hold`, `holdc` and the list functions into every step (41 calls out of the steps become 5). The kernel is header-only, so it was inlined already, and LTO changes it little. Its extra ~20% time with LTO and 2x flash are the price of its guarantees (a wrap-safe 32-bit clock, no globals, each node constructed once), not of the language. The hand-tuned assembly is both the fastest and the smallest. On a laptop the C port runs at ~0.17 µs per user up to 2 million users, ~10-200x faster than Python.

  <details>
  <summary>How the C vs C++ result was found (written up as a short paper: <a href="elevator/measurements/PAPER.md">PAPER.md</a>)</summary>

  | Paper section | In this experiment |
  |---|---|
  | **Hypothesis** | "C++ is slower than C" |
  | **First experiment** | C vs the C++ kernel port: +40%, seemingly confirmed |
  | **Proposed mechanisms** | code size and the instruction cache, kernel overhead, translation-unit boundaries: plausible, mostly wrong |
  | **Falsification attempts** | an inline-ASM `sortin`, `noinline` hot functions, the kernel by value: they disproved the explanations, not the result |
  | **Control** | the C design written in C++: only the language changes |
  | **Confounder found** | `-flto` set on the C folder only: a hidden variable in the setup |
  | **Controlled result** | equal flags: 198 / 198 / 197 cycles per step, and 142 / 140 / 171 with LTO |
  | **Validity checks** | an on-board checksum (`same_run`), golden traces, PC tests under sanitizers |
  | **Threats to validity** | interrupts inside steps, code-placement noise (±1k cycles), one board and one compiler |
  | **Conclusion** | the language costs nothing; LTO and the design's guarantees do |

  </details>

## Algorithm from Concrete Mathematics:
- [Josephus Problem (Generalized)](josephus_generalized)

# Data Structures:
- [Stack](stack)
- [Queue](queue)
- [Circular List](circular_list)
- [XOR Circular List](xor_circular_list)
- [Doubly Linked List](doubly_linked_list)
- [Circle of Buffers](knuth_kernel#circleofbuffers-circle_of_buffershpp) (§1.4.4): Knuth's green, yellow and red buffers between a program and a device, with atomics.
  On the board it carries the UART's DMA transmit and the [elevator's trace](elevator/cpp_embedded_kernel/board/pendsv): printing from the kernel's steps
  took the worst lateness to 604 µs, through the circle it stays at 6 µs, with the same output byte for byte.
  [Tested](knuth_kernel/tests/test_circle_of_buffers.cpp) in CI under AddressSanitizer, UBSan and ThreadSanitizer.

# Systems:
- [Knuth kernel](knuth_kernel): a run-to-completion event kernel built from the elevator's WAIT list (§2.2.5):
  programs are nodes, `HOLD`/`IMMED`/`CYCLE` are the scheduler, nodes created at run time come from an AVAIL-list
  storage pool (§2.2.3). Run to completion within a level (a higher level preempts a lower one), one stack, no heap after startup; tested in CI under AddressSanitizer and UBSan.
  On the STM32G431 it runs at two NVIC priority levels, each with its own WAIT list (14 in a spare interrupt vector, 15 in PendSV),
  woken by TIM2's compares at each deadline ([board app](elevator/cpp_embedded_kernel/board)): max lateness 5 µs, average 0.28 µs,
  as with the earlier single PendSV level (the alarm path takes its level as a template argument).
- [Pi ↔ MCU protocol](pi_mcu_protocol) *(in progress: steps 1–3 of 9)*: a Raspberry Pi ↔ STM32 serial link built from TAOCP ideas:
  a lock-free circular queue (§2.2.2) between the UART interrupt and the main loop, CRC-framed lines,
  and a [circle of buffers](knuth_kernel#circleofbuffers-circle_of_buffershpp) (§1.4.4) for DMA transmit, one whole line per transfer.
  C++23 on the MCU, Python on the Pi, tested in CI under AddressSanitizer, UBSan and ThreadSanitizer.

# Firmware:
- [Knuth elevator](firmware/kernel_elevator): the C++ elevator on the [Knuth kernel](knuth_kernel) for the NUCLEO-G431RB,
  One script (`./setup.sh test`) fetches CMSIS, builds with CMake,
  flashes the board and checks the run: 544 steps, the same checksum as the simulation, the CPU asleep 99.99% of the time,
  and an MPU guard at the bottom of the stack that turns an overflow into a recorded HardFault.

# Papers:
- [Is C++ Slower than C on a Cortex-M4? A Controlled Measurement with Knuth's Elevator](elevator/measurements/PAPER.md):
  the elevator implemented three ways (C, the C design written in C++, and C++ on the [Knuth kernel](knuth_kernel)),
  measured with the DWT cycle counter on the NUCLEO-G431RB, with an on-board checksum proving all three run the same 544 events.
  A first comparison showed C++ 40% slower; a control version exposed the cause, `-flto` enabled for the C code only.
  With identical flags C and C++ run at the same speed (198 vs 198 cycles per event, 142 vs 140 with LTO);
  the general kernel costs ~20% more under LTO, the price of its guarantees, not of the language.
  Sections: abstract, method, results, the confounder and the hypotheses it disproved, threats to validity;
  the full run history is in [RESULTS.md](elevator/measurements/RESULTS.md#history-c-vs-c-runs-in-order).
  To run it yourself: [the board harness](elevator/measurements/board) (`main.c` and how to set up the CubeIDE project).


---

*Claude (Opus 5.5) assistance:*
- *Drafted all READMEs, SCHEME files and tests;*
- *Ran the board tests and measurements (DWT, GDB, `setup.sh`) and reviewed the code.*
- *Suggested, wrote and then proved that to_chars is faster than snprintf in trace;*
- *Found, in the `format_trace` assembly I asked for, that libstdc++'s `to_chars` copies its 201-byte digit table onto the stack with `memcpy` for every number of 10 or more; with the digits written by hand the trace formats 2.6 times faster (3 740 → 1 430 cycles a line), with no table on the stack (the frame 280 → 56 bytes) and 624 bytes less flash;*
- *Suggested to use template for kernel levels, for splitting compile/runtime code, then proved that templates provided some hints to compiler for optimizing kernel levels API usage: average lateness 0.77µs → 0.28 µs, at the cost of +288 bytes of flash.*
- *Investigated why the G431 skipped an instruction after the masked `__WFI` with `DBG_SLEEP` (ES0431 §2.2.7, seen even with ST's workarounds), and moved `kernel_port_sleep()` to SRAM; checked in 20 code layouts, since the skip appeared only in layouts with `wfi` at an address ≡ 4 mod 8 (not in all of them).*
- *That became possible after I insisted on a CMake build. In one of the later runs Claude flashed the CMake build to the MCU with the Debug settings and noticed that `idle_us` showed a garbage value, while the steps and the checksum were right; since then `./setup.sh test` also checks the wakes and `idle_us`. I asked Claude to investigate the cause and to give me links to the related documentation. I checked the documentation and told Claude to fix the bug accordingly.*
- *Tested the stack guard on the board, with a deliberate overflow and the fault record, and wrote the static stack estimate, which matches the measured 228 bytes.*
- *Suggested one Kernel per priority level for the kernel levels (stage 2); I later replaced it with one Kernel per MCU, holding a WAIT list for each level*
- *When a level had to wake a node on another level, suggested an events circle per poster, which is a bad design: a circle for every pair. I made each node know its level instead, so every operation goes to the node's own list.*
- *Pointed that set_priority on a waiting node breaks rule 1: The node sits in list 14. If its priority then becomes 15, the next hold unlinks it from list 14 using list 15's code, run from level 15.*
- *Suggested to use templates and constexprs in Kernel.*
- *Wrote the CI job that cross-compiles the STM32G4 port and the bare firmware with -Werror, the only place CI builds the port, and checked that a wrong level table fails it.*
