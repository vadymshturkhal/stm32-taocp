# The Art of Computer Programming on STM32: C, C++, Python and ARM Assembly

[![Elevator (Python)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-python.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-python.yml)
[![Elevator (C)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-c.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/elevator-c.yml)
[![Pi-MCU protocol](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/pi-mcu-protocol.yml/badge.svg)](https://github.com/vadymshturkhal/stm32-taocp/actions/workflows/pi-mcu-protocol.yml)

- This repository contains Python, C, C++ and hand-tuned ARM Assembly implementations of [Algorithms](#algorithms) and [Data Structures](#data-structures) from The Art of Computer Programming, Vol.1, and a [Raspberry Pi ↔ STM32 protocol](#systems) built from its ideas.

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
- [Elevator: discrete simulation program](elevator) ([Python](elevator/Python_WAIT_LIST), [C](elevator/C_Embedded), [C (native)](elevator/C_Native), [ARM Assembly](elevator/Assembly), [Python (async)](elevator/Python_async), [C++ on the Knuth kernel](elevator/Cpp_Embedded_Kernel), [Measurements](elevator/Measurements))
  <details>
  <summary>Knuth's MIX elevator simulation (TAOCP Vol. 1, 2.2.5), implemented across languages and targets</summary>

  - [Python](elevator/Python_WAIT_LIST): subroutine-based port driven by an explicit WAIT list, without `yield`/`async`
  - [C](elevator/C_Embedded): embedded-target port for the STM32G4, paired with the ARM Assembly port below
  - [C (native)](elevator/C_Native): standalone laptop build (malloc-based, no STM32 dependencies), used for timing comparisons against Python
  - [ARM Assembly](elevator/Assembly): hand-tuned Cortex-M4 Thumb-2 port, paired with the embedded C port above
  - [Python (async)](elevator/Python_async): coroutine-based port using `asyncio`/`await` instead of an explicit WAIT list
  - [C++ on the Knuth kernel](elevator/Cpp_Embedded_Kernel) *(in progress)*: the 1968 discrete-event design running, unchanged in spirit, as the scheduler of a real microcontroller: the steps E1–E9 and U1–U6 are nodes of the [Knuth kernel](knuth_kernel)'s WAIT list, with a trace that can be checked against the book
  - [Measurements](elevator/Measurements): scripts comparing timing and trace-output correctness between the C (native) and Python ports -- see [RESULTS.md](elevator/Measurements/RESULTS.md) for measured numbers (ASM vs GCC on STM32, C vs Python on laptop)
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
- [Pi ↔ MCU protocol](pi_mcu_protocol) *(in progress: steps 1–3 of 9)*: a Raspberry Pi ↔ STM32 serial link built from TAOCP ideas:
  a lock-free circular queue (§2.2.2) between the UART interrupt and the main loop, CRC-framed lines,
  and next a circle of buffers (§1.4.4) for zero-copy DMA transmit.
  C++23 on the MCU, Python on the Pi, tested in CI under AddressSanitizer, UBSan and ThreadSanitizer.
