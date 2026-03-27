# Implementation of The Art of Computer Programming Vol.1 in C and ARM Assembly

# Implemented algorithms:
- Algorithm P (Print table of 500 primes);
- Algorithm A (Multiply permutations in cycle form);
- Algorithm B (Multiply permutations in cycle form);
- Algorithm I (Inverse permutation in place);
- Algorithm J (Inverse permutation in place);
- Josephus Problem (Generalized);
- Stack;

## How to run:
- Navigate to the specific algorithm's folder;
- Run the C script starting with the word `comparing`;
- Note: The latest ARM Assembly versions have the highest trailing number in their filename (e.g., `asm_get_auxiliary_table4.s` is the fully optimized version).

## Performance summary GCC -O3 and ARM Assembly:
### All cycle counts measured using the internal DWT cycle counter on the Cortex-M4

<details>
<summary><b>Algorithm P (Print table of 500 primes)</b></summary>

* **Base case = `500 primes`:**
    * GCC -O3: cycles_cold = [120736-120741], cycles_warm = [120712-120713], size = 76 bytes
    * ARM Assembly: cycles_cold = [115679-115712], cycles_warm = [115654-115692], size = 72 bytes
    * **Summary:** Hand-tuned ASM won by ~5,057 cycles (**~4.1% time reduction**) in the cold version and by ~5,058 cycles (**~4.1% time reduction**) in the warm one,    with ARM Assembly consuming **~5.2%** less Flash memory
</details>

<details>
<summary><b>Algorithm A (Multiply permutations in cycle form)</b></summary>

* **Base case = `"(acfg)(bcd)(aed)(fade)(bgfae)"`:**
    * GCC -O3: cycles_cold = [2977-2983], cycles warm = 2917, size = 236 bytes
    * ARM Assembly: cycles_cold = [2613-2618], cycles warm = 2547, size = 268 bytes
    * **Summary:** Hand-tuned ASM won by ~364 cycles (**~12.2% time reduction**) in the cold version and by ~370 cycles (**~12.6% time reduction**) in the warm one, with GCC consuming **~11.9%** less Flash memory due to aggressive compiler inlining
</details>


<details>
<summary><b>Algorithm B (Multiply permutations in cycle form)</b></summary>

* **Base case = `"(acfg)(bcd)(aed)(fade)(bgfae)"`:**
    * **Using parallel arrays as a table:**
        * GCC -O3: cycles_cold = [1968-1970], cycles_warm = 1893, size = 296 bytes
        * ARM Assembly: cycles_cold = [1741-1747], cycles_warm = 1662, size = 322 bytes
        * **Summary:** Hand-tuned ASM won by ~227 cycles (**~11.5% time reduction**) in the cold version and by ~231 cycles (**~12.2% time reduction**) in the warm one, with GCC consuming **~8%** less Flash memory
        * **Some tricks and insights:** The "Silicon Butterfly Effect" & Cache-Line Shields (NOP, .balign), The ABI bypass (Removed C-Returns), Instructions sizing
            as Physical Tetris (16-bit vs 32-bit), The Bare-Metal VLA (Variable Length Array) Hack (stack with BIC instead of malloc), 
            Write-Back Hazard Avoidance (TST instead of AND+CBZ), Pre-increment Pointer Hack (for avoiding late SUB)

    * **Using one array as a table:**
        * GCC -O3: cycles_cold = [947-959], cycles_warm = [887-890], size = 260 bytes
        * ARM Assembly: cycles_cold = [778-784], cycles_warm = 736, size = 224 bytes
        * **Summary:** Hand-tuned ASM won by ~169 cycles (**~17.8% time reduction**) in the cold version and by ~151 cycles (**~17.0% time reduction**) in the warm one, with ARM Assembly consuming **~13.8%** less Flash memory
        * **Some tricks and insights:** Pointer Shifting, Hot Path Isolation & Register Recycling, Free Bounds Checking, 16-bit Thumb-2 instructions instead of standard 32-bit instructions, Branchless Conditional Logic
</details>

<details>
<summary><b>Algorithm I (Inverse in place)</b></summary>

* **Base case = `{0, 6, 2, 1, 5, 4, 3}`:**
    * GCC -O3: cycles_cold = [181-189], cycles_warm = 167, size = 100 bytes
    * ARM Assembly: cycles_cold = 151, cycles_warm = 139, size = 50 bytes
    * **Summary:** Hand-tuned ASM won by ~30 cycles (**~16.6% time reduction**) in the cold version and by ~28 (**~16.7% time reduction**) in the warm one, with ARM Assembly consuming **50%** less Flash memory
</details>

<details>
<summary><b>Algorithm J (Inverse in place)</b></summary>

* **Base case = `{0, 6, 2, 1, 5, 4, 3}`:**
    * GCC -O3: cycles_cold = [225-226], cycles_warm = 203, size = 84 bytes
    * ARM Assembly: cycles_cold = [201-202], cycles_warm = 186, size = 60 bytes
    * **Summary:** Hand-tuned ASM won by ~24 cycles (**~10.6% time reduction**) in the cold version and by ~17 cycles (**~8.3% time reduction**) in the warm one, with ARM Assembly consuming **~28.5%** less Flash memory
</details>

<details>
<summary><b>Josephus Problem (Generalized)</b></summary>

* **Base case = `mod 33 and participants 10111`:**
    * GCC -O3: cycles_cold = 14006, cycles_warm = 13950, size = 92 bytes
    * ARM Assembly: cycles_cold = 4846, cycles_warm = 4816, size = 114 bytes
    * **Summary:** Hand-tuned ASM won by ~9,160 cycles (**~65.4% time reduction**) in the cold version and by ~9,134 cycles (**~65.4% time reduction**) in the warm one, with GCC consuming **~19.2%** less Flash memory
    * **Some tricks and insights:** ASM custom ceiling
</details>

<details>
<summary><b>Stack</b></summary>

* **Base case = `128 nodes, 128 Push and 128 Pop using balloc (custom bare-metal memory allocator)`:**
    * **Translation Unit Boundary Push/Pop case (not inline integrated):**
        * GCC -O3: cycles_cold = 9382, cycles_warm = 9309, size = 420 bytes
        * ARM Assembly: cycles_cold = [6765-6804], cycles_warm = [6726-6745], size = 240 bytes
        * **Summary:** Hand-tuned ASM won by ~2,617 cycles (**~27.8% time reduction**) in the cold version and by ~2,583 cycles (**~28.1% time reduction**) in the warm one, with ASM consuming **~42.8%** less Flash memory
        * **Some tricks and insights:** 
        Aggressive use of Scratch Registers: Exploiting the lack of branches to safely place the Stack pointer in a scratch register across 256 loop iterations, 
        Register-level flag usage for error handling, 
        Strict AAPCS 8-byte Stack Alignment, 
        16-bit Thumb-2 Density: Forcing all operations in low registers, 
        Instruction Pipeline Alignment (.balign 4), 
        Cascade Return Architecture: Fall-through error handling to minimize epilogue redundancy,
        Custom bare-metal memory allocator (balloc),

    * **Inlined Push/Pop (integrated):**
        * GCC -O3: cycles_cold = [4226-4231], cycles_warm = [4172-4173], size = 200 bytes
        * ARM Assembly: cycles_cold = [2615-2626], cycles_warm = [2554-2555], size = 390 bytes
        * **Summary:** Hand-tuned ASM won by ~1611 cycles (**~38.1% time reduction**) in the cold run and by ~1618 cycles (**~38.7% time reduction**) in the warm run, with GCC consuming **48.7%** less Flash memory;
        * **Some tricks and insights:** 
        16-bit Thumb-2 Density: Forcing all operations in low registers,
        Instruction Pipeline Alignment (.balign 4),
        Bump Allocator: balloc,
        Reduced SRAM traffic by hoisting Top and Avail nodes,
        Loop Unrolling & Modulo Variable Expansion (MVE mod 4): the register permutation returns to identity after 4 cycles
</details>