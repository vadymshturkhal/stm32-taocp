#include "main.h"

typedef struct CircularNode {
    uint32_t info;
    struct CircularNode* link;
} CircularNode;

typedef struct {
	CircularNode* ptr;
	CircularNode* avail;
} CircularList;

// Prototypes
extern void* asm_balloc(uint32_t size);
extern void asm_balloc_free(void* memory_pointer);

CircularList* c_create_circular_list(void* memory, uint32_t nodes);
extern clang_asm_circular_list_init(void* memory, uint32_t nodes);
extern asm_create_circular_list_lipski_mod4(void* memory, uint32_t nodes);
extern rust_asm_circular_list_init(void* memory, uint32_t nodes);


void comparing_circular_list_init() {
	// notice that stack node must be 8 bytes long;

    volatile uint32_t start, end, overhead;

	start = DWT->CYCCNT;
	end = DWT->CYCCNT;
	overhead = end - start;

	const uint32_t max_nodes = 512;
	void* memory1 = asm_balloc(max_nodes * sizeof(CircularNode) + sizeof(CircularList));
	if (memory1 == NULL) return 0;
	void* memory2 = asm_balloc(max_nodes * sizeof(CircularNode) + sizeof(CircularList));
	if (memory2 == NULL) return 0;

	// GCC -O3 -mcpu=cortex-m4 -mthumb: cycles_cold = 1730, cycles_warm = 1691, size = 112 bytes
	start = DWT->CYCCNT;
	CircularList* circular_list = c_create_circular_list(memory1, max_nodes);
	end = DWT->CYCCNT;
	volatile uint32_t gcc_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	circular_list = c_create_circular_list(memory1, max_nodes);
	end = DWT->CYCCNT;
	volatile uint32_t gcc_cycles_warm = (end - start) - overhead;

	// Clang -O3 -mcpu=cortex-m4 -mthumb --target=arm-none-eabi:
	// cycles_cold = 1471-1478, cycles_warm = 1440-1445, size = 244 bytes
	start = DWT->CYCCNT;
	CircularList* clang_circular_list = clang_asm_circular_list_init(memory2, max_nodes);
	end = DWT->CYCCNT;
	volatile uint32_t clang_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	clang_circular_list = clang_asm_circular_list_init(memory2, max_nodes);
	end = DWT->CYCCNT;
	volatile uint32_t clang_cycles_warm = (end - start) - overhead;

	// ARM Assembly: cycles_cold = 1458-1465, cycles_warm = 1439-1440, size = 100 bytes
	start = DWT->CYCCNT;
	CircularList* asm_circular_list = asm_create_circular_list_lipski_mod4(memory1, max_nodes);
	end = DWT->CYCCNT;
	volatile uint32_t asm_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	asm_circular_list = asm_create_circular_list_lipski_mod4(memory1, max_nodes);
	end = DWT->CYCCNT;
	volatile uint32_t asm_cycles_warm = (end - start) - overhead;

	// Rust --target thumbv7em-none-eabi -C opt-level=3 -C target-cpu=cortex-m4
	// cycles_cold = 1480-1488, cycles_warm = 1441-1446, size = 128 bytes
	start = DWT->CYCCNT;
	CircularList* rust_circular_list = rust_asm_circular_list_init(memory2, max_nodes);
	end = DWT->CYCCNT;
	volatile uint32_t rust_cycles_cold = (end - start) - overhead;

	start = DWT->CYCCNT;
	rust_circular_list = rust_asm_circular_list_init(memory2, max_nodes);
	end = DWT->CYCCNT;
	volatile uint32_t rust_cycles_warm = (end - start) - overhead;


	// free all memory at once
	asm_balloc_free(memory1);
}
