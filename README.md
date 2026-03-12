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
        * GCC -O3: cycles_cold = [1984-1996], cycles_warm = 1913;
        * ARM Assembly: cycles_cold = [1875-1877], cycles_warm = 1790;
        * **Summary:** Hand-tuned ASM won by ~109 cycles (**~5.5% time reduction**) in the cold version and by ~123 cycles (**~6.4% time reduction**) in the warm one;

    * **Using one array as a table:**
        * GCC -O3: cycles_cold = [947-959], cycles_warm = [887-890];
        * ARM Assembly: cycles_cold = [778-784], cycles_warm = 736;
        * **Summary:** Hand-tuned ASM won by ~169 cycles (**~17.8% time reduction**) in the cold version and by ~151 cycles (**~17.0% time reduction**) in the warm one;
</details>

<details>
<summary><b>Algorithm I (Inverse in place)</b></summary>

* **Base case = `{0, 6, 2, 1, 5, 4, 3}`:**
    * GCC -O3: cycles_cold = [181-189], cycles_warm = 167, size = 100 bytes;
    * ARM Assembly: cycles_cold = 151, cycles_warm = 139, size = 50 bytes;
    * **Summary:** Hand-tuned ASM won by ~30 cycles (**~16.6% time reduction**) in the cold version and by ~28 (**~16.7% time reduction**) in the warm one, while consuming **50%** less Flash memory;
</details>
