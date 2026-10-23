# Is C++ Slower than C on a Cortex-M4? A Controlled Measurement with Knuth's Elevator

*Vadym Shturkhal. Measurements on a NUCLEO-G431RB; data and code in this repository. (Claude helped to write this summary.)*

## Abstract

Embedded developers often assume that C++ costs speed and flash compared with C.
We test that assumption on a Cortex-M4 (STM32G431, 170 MHz) using Knuth's
elevator simulation (TAOCP Vol. 1, §2.2.5), implemented three ways: in C, in
C++ with the C design kept 1:1, and in C++ on a general run-to-completion
kernel. A first comparison showed C++ 40% slower. Controlled runs showed that
the gap came from a build flag: link-time optimization (`-flto`) was enabled for
the C code only. With identical flags, C and C-style C++ run at the same speed
(198 vs 198 cycles per step without LTO, 142 vs 140 with LTO). The general
kernel costs about 20% more per step under LTO, which is the price of its
guarantees, not of the language. An on-board checksum confirms that all three
versions schedule the same 544 events in the same order.

## 1. Introduction

The question "is C++ slower than C?" is usually answered from experience with
features that embedded firmware avoids: exceptions, RTTI, virtual dispatch,
`iostream`, and heap allocation. We ask a narrower, measurable question: for the
same algorithm on the same microcontroller and compiler, does writing it in C++
change the cycle count, and if not, what does?

The workload is a discrete-event simulation: 66 users and one elevator,
scheduled through a time-sorted doubly linked list (Knuth's WAIT list). It is
dominated by pointer chasing, sorted insertion and indirect calls, which makes it
typical of event-driven firmware.

## 2. Systems under test

| Version | Design |
|---|---|
| **C** ([`c_embedded`](../c_embedded)) | one `malloc`'d block; a `SharedState*` passed to every function; Knuth's sentinel (the list head's time is 0) with unsigned compares; the scheduler (`hold`, `sortin`) in its own `.c` file |
| **C-style C++** ([`cpp_cstyle`](../cpp_cstyle)) | the C version ported 1:1: the same structs, files, calls and compares; only zero-cost spellings differ (`namespace`, `constexpr`, `nullptr`, `static_cast`) |
| **Kernel C++** ([`cpp_embedded_kernel`](../cpp_embedded_kernel)) | the steps as nodes of a general header-only kernel ([`knuth_kernel`](../../knuth_kernel)): a wrap-safe 32-bit clock, no globals (each node points back to its program), `hold` always restarts a node, a typed storage pool |

All three produce byte-identical traces on the PC against the same golden files,
checked in CI.

## 3. Method

- **Hardware and tools:** STM32G431RB (Cortex-M4F, 170 MHz, flash with 4 wait
  states), GNU Tools for STM32 14.3, `-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16
  -mfloat-abi=hard`, C++ with `-fno-exceptions -fno-rtti`, `NDEBUG`.
- **Harness:** `main()` runs the three versions one after another. Each runs **one
  event per main-loop pass**, beside a live UART protocol (`Link_Poll`), as fast
  as possible (each event at its own simulated time).
- **Measure:** the DWT cycle counter around every event, minus the cost of two
  counter reads; the setup (allocation and initialization) is timed separately.
- **Workload:** 66 users from a fixed seed, **544 events** per version.
- **Equivalence check:** after every event, `sum = sum × 31 + TIME`. The three
  checksums must be equal (`same_run = 1`); this held in every run reported.
- **Controlled variable:** the optimization flags of each source folder, read
  back from the generated makefiles (`subdir.mk`), not from the settings dialog.

## 4. Results

**Cycles per event, identical flags for all three folders:**

| Flags | C | C-style C++ | Kernel C++ |
|---|---|---|---|
| `-O3` | 198 | **198** | 197 |
| `-O3 -flto` | 142 | **140** | 171 |

**Whole run, `-O3 -flto`:**

| | C | C-style C++ | Kernel C++ |
|---|---|---|---|
| Events (cycles) | 77 656 | 76 700 | 93 442 |
| Setup (cycles) | 1 555 | 1 166 | 6 847 |
| Flash (elevator objects, `-O3`) | ~2.7 KB | ~3.0 KB | ~5.5 KB |

At `-O0` the kernel version took 2 385 cycles per event, 12x its `-O3` speed:
C++ abstractions are free only with the optimizer on.

## 5. The confounder

The first comparisons (C at 142–146 cycles per event against the kernel C++ at
201–216) suggested C++ was 40% slower. We proposed and tested three mechanisms:

| Hypothesis | Test | Result |
|---|---|---|
| The sorted-insert loop is the hot spot | a wrap-safe sentinel (the head takes the new node's time, removing one compare per node) | **−11 cycles/event, kept** |
| The loop needs hand-scheduled assembly | an inline-assembly `sortin` modelled on a hand-tuned ASM port | no change, reverted |
| Inlining duplicates hot code and thrashes the small instruction cache | `noinline` on `hold`/`sortin` | **slower** (+15), reverted |
| The kernel reference costs a load | the kernel owned by value | −3, reverted (it breaks sharing) |

None explained the gap. The control version (C-style C++) then measured 196
against C's 146, although its source was the C design. Reading the generated
makefiles showed the cause: **the C folder was compiled with `-flto`, the C++
folders were not**. LTO inlines `hold` and `sortin` across `.c` files; without it
they are calls. With the flags equalized (Section 4) the language difference
disappeared.

## 6. Discussion

- **The language.** GCC compiles C and C++ through the same optimizer and back
  end. Once classes, references and templates are lowered, the same design gives
  the same instructions; the measurements agree to within 1%.
- **The kernel's cost.** Without LTO, the header-only kernel is already inlined,
  which offsets its extra work (197 vs 198). With LTO the C designs gain the same
  inlining, and the kernel's guarantees show: a two-instruction wrap-safe compare
  per node passed, an unlink before every insert, a back-pointer load, a time
  conversion, and resetting removed nodes. Together about 30 cycles per event.
- **Setup and size.** The kernel version constructs every object on each run
  (once at boot in real firmware) and is twice as large in flash, because its
  scheduler is inlined into every step.

## 7. Threats to validity

- **Interrupts** (SysTick, UART, DMA) that land inside an event are counted. They
  affect all three versions alike, but add noise.
- **Code placement:** unchanged code moved by about ±1k cycles per run (out of
  ~80–110k) when other code changed, from flash alignment and prefetch effects.
  Differences below that are not significant.
- **Scope:** one board, one compiler version, one workload. Other cores (with
  caches or branch predictors), other compilers, or code relying on exceptions
  and virtual dispatch may behave differently.
- **Early runs** used inconsistent flags; they are reported in the run history
  ([RESULTS.md](RESULTS.md#history-c-vs-c-runs-in-order)) but not used for the
  conclusions.

## 8. Conclusion

For the same design on a Cortex-M4 with GCC, **C++ costs nothing compared with
C**. The large effects came from the build (LTO: −28% for the C designs) and from
the design (a general kernel's guarantees: +20% under LTO; constructing objects
and inlining everywhere: more setup and twice the flash). The practical lesson
is about method: **compare builds only after verifying the flags every folder
was really compiled with, and prove the versions do the same work.** For a
dozen runs, an uncontrolled flag looked like a property of the language.

## Reproducibility

- Code: [`c_embedded`](../c_embedded), [`cpp_cstyle`](../cpp_cstyle),
  [`cpp_embedded_kernel`](../cpp_embedded_kernel), [`knuth_kernel`](../../knuth_kernel).
- PC correctness: each port's README; CI diffs all five golden traces
  ([`measurements`](.)), also under AddressSanitizer and UBSan.
- Board numbers: the CubeIDE project's `main()` ([`board/`](board), with setup
  and run instructions) runs the three versions and collects every count in
  `elevator_finish()`; the full run history, including
  the misleading runs, is in [RESULTS.md](RESULTS.md).
