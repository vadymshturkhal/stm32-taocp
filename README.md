# Implementation of The Art of Computer Programming Vol.1 in C and ARM Assembly

# Implemented algorithms:
- Algorithm P (Print table of 500 primes);
- Algorithm A (Multiply permutations in cycle form);
- Algorithm B (Multiply permutations in cycle form);
- Algorithm I (Inverse permutation in place);

## How to run:
- Navigate to the specific algorithm's folder;
- Run the C script starting with the word `comparing`;
- Note: The latest ARM Assembly versions have the highest trailing number in their filename (e.g., `asm_get_auxiliary_table4.s` is the fully optimized version).

## Performance summary GCC -O3 and ARM Assembly:
### All cycle counts measured using the internal DWT cycle counter on the Cortex-M4
- Algorithm P with 500 primes:
    - GCC -O3: cycles_cold = [167241-167611], cycles_warm = 167254;
    - ARM Assembly: cycles_cold = [158090-158300], cycles_warm = 158034;
    - Summary: Hand-tuned ASM won by ~9,151 cycles in the cold version and by ~9,220 cycles in the warm one;

<details>
<summary><b>Algorithm A (Multiply permutations in cycle form)</b></summary>

* **Base case = `"(acfg)(bcd)(aed)(fade)(bgfae)"`:**
    * GCC -O3: cycles_cold = [2977-2983], cycles warm = 2917, size = 236 bytes;
    * ARM Assembly: cycles_cold = [2613-2618], cycles warm = 2547, size = 268 bytes;
    * **Summary:** Hand-tuned ASM won by ~364 cycles (**~12.2% time reduction**) in the cold version and by ~370 cycles (**~12.6% time reduction**) in the warm one, with GCC consuming **11.9%** less Flash memory due to aggressive compiler inlining;
</details>


<details>
<summary><b>Algorithm B (Multiply permutations in cycle form)</b></summary>

* **Base case = `"(acfg)(bcd)(aed)(fade)(bgfae)"`:**
    * **Using parallel arrays as a table:**
        * GCC -O3: cycles_cold = [1968-1970], cycles_warm = 1893, size = 296 bytes;
        * ARM Assembly: cycles_cold = [1741-1747], cycles_warm = 1662, size = 322;
        * **Summary:** Hand-tuned ASM won by ~227 cycles (**~11.5% time reduction**) in the cold version and by ~231 cycles (**~12.2% time reduction**) in the warm one, with GCC consuming **8%** less Flash memory;
        * **Some tricks and insights:** The "Silicon Butterfly Effect" & Cache-Line Shields (NOP, .balign), The ABI bypass (Removed C-Returns), Instructions sizing
            as Physical Tetris (16-bit vs 32-bit), The Bare-Metal VLA (Variable Length Array) Hack (stack with BIC instead of malloc), 
            Write-Back Hazard Avoidance (TST instead of AND+CBZ), Pre-increment Pointer Hack (for avoiding late SUB);

    * **Using one array as a table:**
        * GCC -O3: cycles_cold = [947-959], cycles_warm = [887-890], size = 260 bytes;
        * ARM Assembly: cycles_cold = [778-784], cycles_warm = 736, size = 224 bytes;
        * **Summary:** Hand-tuned ASM won by ~169 cycles (**~17.8% time reduction**) in the cold version and by ~151 cycles (**~17.0% time reduction**) in the warm one, with ARM Assembly consuming **13.8%** less Flash memory;
</details>

<details>
<summary><b>Algorithm I (Inverse in place)</b></summary>

* **Base case = `{0, 6, 2, 1, 5, 4, 3}`:**
    * GCC -O3: cycles_cold = [181-189], cycles_warm = 167, size = 100 bytes;
    * ARM Assembly: cycles_cold = 151, cycles_warm = 139, size = 50 bytes;
    * **Summary:** Hand-tuned ASM won by ~30 cycles (**~16.6% time reduction**) in the cold version and by ~28 (**~16.7% time reduction**) in the warm one, with ARM Assembly consuming **50%** less Flash memory;
</details>
