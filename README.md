# Implementation of The Art of Computer Programming Vol.1 in C and ARM Assembly

# Implemented algorithms:
- Algorithm P (Print table of 500 primes);
- Algorithm A (Multiply permutations in cycle form);
- Algorithm B (Multiply permutations in cycle form);

## How to run:
- Navigate to the specific algorithm's folder;
- Run the C script starting with the word `comparing`;
- Note: The latest ARM Assembly versions have the highest trailing number in their filename (e.g., `asm_get_auxiliary_table4.s` is the fully optimized version).

## Performance summary GCC -O3 and ARM Assembly:
### All cycle counts measured using the internal DWT cycle counter on the Cortex-M4
- Algorithm P for 500 primes:
    - GCC -O3: cycles_cold = [167241-167611], cycles_warm = 167254
    - ARM Assembly: cycles_cold = [158090-158300], cycles_warm = 158034;
    - Summary: Hand-tuned ASM won by ~9,151 cycles in the cold version and by ~9,220 cycles in the warm one;

- Algorithm A with base case = "(acfg)(bcd)(aed)(fade)(bgfae)":
    - GCC -O3: cycles cold = [2978-2983], cycles warm = 2917;
    - ARM Assembly: cycles cold = [2958-2963], cycles warm = [2874-2875];
    - Summary: Hand-tuned ASM won by ~20 cycles in the cold version and by ~43 cycles in the warm one;

- Algorithm B with base case = "(acfg)(bcd)(aed)(fade)(bgfae):
    - Parallel arrays as a table version:
        - GCC -O3: cycles_cold = [1984-1996], cycles_warm = 1913;
        - ARM Assembly: cycles_cold = [1875-1877], cycles_warm = 1790;
        - Summary: Hand-tuned ASM won by ~109 cycles in the cold version and by ~123 in the warm one;

