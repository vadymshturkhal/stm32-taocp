# The Art of Computer Programming on STM32: C, C++, Python and ARM Assembly

[![Elevator (Python)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-python.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-python.yml)
[![Elevator (C)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-c.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-c.yml)
[![Elevator (C++ on the Knuth kernel)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-cpp.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-cpp.yml)
[![Elevator (C-style C++)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-cpp-cstyle.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-cpp-cstyle.yml)
[![Knuth kernel](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/knuth-kernel.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/knuth-kernel.yml)
[![Pi-MCU protocol](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/pi-mcu-protocol.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/pi-mcu-protocol.yml)

- This repository contains Python, C, C++ and hand-tuned ARM Assembly implementations of [Algorithms](#algorithms) and [Data Structures](#data-structures) from The Art of Computer Programming, Vol.1, and a [Raspberry Pi ↔ STM32 protocol](#systems) built from its ideas.
- [Knuth kernel](#systems): Knuth's 1968 CYCLE running as a real-time kernel on the Cortex-M4, in PendSV woken by a hardware timer: every event within 6 µs of its deadline, the CPU asleep 99.8% of the time, the same result as the simulation to the last bit.
- [Paper](#papers): *Is C++ Slower than C on a Cortex-M4?* A controlled measurement on the board: the same design costs the same in both languages.

## Why this repository exists

I planned to work through TAOCP in Python. Then I met MIX, Knuth's assembly
language for an imaginary computer, and almost stopped. I kept going instead, and
MIX turned out to be the bridge: it led to hand-written ARM assembly on a real
microcontroller (now faster and smaller than GCC's best), then to C and C++
versions checked against the same output, and finally to Knuth's 1968 elevator
scheduler running as a real-time kernel on the board. Knuth keeps MIX in the
books so that readers understand what the machine really does; this repository
is what came of taking that seriously.

## Hardware and tools

| What | Used |
|------|------|
| Board | NUCLEO-G431RB: STM32G431RB, Arm Cortex-M4 with FPU, 170 MHz, 128 KB flash, 32 KB RAM |
| MCU toolchain | STM32CubeIDE, `arm-none-eabi-gcc` / `g++` 14.3 (GNU Tools for STM32) |
| PC builds and tests | `gcc`, `g++-14` (C++23), Python 3.10+ |
| Host for the protocol | Raspberry Pi 4 (a Linux PC over the ST-LINK USB serial port during development) |
| CI | GitHub Actions: the badges above; each workflow builds and tests one project on every push |

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

# Systems:
- [Knuth kernel](knuth_kernel): a run-to-completion event kernel built from the elevator's WAIT list (§2.2.5):
  programs are nodes, `HOLD`/`IMMED`/`CYCLE` are the scheduler, nodes created at run time come from an AVAIL-list
  storage pool (§2.2.3). No preemption, one stack, no heap after startup; tested in CI under AddressSanitizer and UBSan.
  On the STM32G431 it runs in PendSV, woken by the TIM2 compare at each deadline ([board app](elevator/cpp_embedded_kernel/board)):
  max lateness 6 µs, average 301 ns.
- [Pi ↔ MCU protocol](pi_mcu_protocol) *(in progress: steps 1–3 of 9)*: a Raspberry Pi ↔ STM32 serial link built from TAOCP ideas:
  a lock-free circular queue (§2.2.2) between the UART interrupt and the main loop, CRC-framed lines,
  and next a circle of buffers (§1.4.4) for zero-copy DMA transmit.
  C++23 on the MCU, Python on the Pi, tested in CI under AddressSanitizer, UBSan and ThreadSanitizer.

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

*Code, design and measurements are mine; Claude helped draft the READMEs and the paper, and reviewed code.*
