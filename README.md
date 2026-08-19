# Implementation of The Art of Computer Programming Vol.1 in C and ARM Assembly
- This repository contains Python, C, and hand-tuned ARM Assembly implementations of [Algorithms](#algorithms) and [Data Structures](#data-structures) from The Art of Computer Programming, Vol.1 

# Algorithms:
- [Algorithm P (Print table of 500 primes)](knuth_primes)
- [Algorithm A (Multiply permutations in cycle form)](multiply_permutations_a)
- [Algorithm B (Multiply permutations in cycle form)](multiply_permutations_b)
- [Algorithm I (Inverse permutation in place)](inverse_permutation_in_place)
- [Algorithm J (Inverse permutation in place)](inverse_permutation_in_place_j)
- [Algorithm T (Topological Sort)](topological_sort)
- [Algorithm A (Addition of polynomials)](polynomials/addition_of_polynomials)
- [Algorithm M (Multiplication of polynomials)](polynomials/multiplication_of_polynomials)
- [Elevator: discrete simulation program](elevator) ([Python](elevator/Python_WAIT_LIST), [C](elevator/C_WAIT_LIST), [C (native)](elevator/C_Native), [ARM Assembly](elevator/ASM_WAIT_LIST), [Python (async)](elevator/Python_async), [Measurements](elevator/Measurements))
  <details>
  <summary>Knuth's MIX elevator simulation (TAOCP Vol. 1, 2.2.5), implemented across languages and targets</summary>

  - [Python](elevator/Python_WAIT_LIST): subroutine-based port driven by an explicit WAIT list, without `yield`/`async`
  - [C](elevator/C_WAIT_LIST): embedded-target port for the STM32G4, paired with the ARM Assembly port below
  - [C (native)](elevator/C_Native): standalone laptop build (malloc-based, no STM32 dependencies), used for timing comparisons against Python
  - [ARM Assembly](elevator/ASM_WAIT_LIST): hand-tuned Cortex-M4 Thumb-2 port, paired with the embedded C port above
  - [Python (async)](elevator/Python_async): coroutine-based port using `asyncio`/`await` instead of an explicit WAIT list
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
