.section .text
    .thumb
    .cpu cortex-m4
	.global asm_balloc_free


@ Free Bump Allocator or Arena Allocator

@ use asm_balloc_free with the earliest address:
@ for example:
@ void* asm_stack_memory = asm_balloc(max_nodes * sizeof(Node) + sizeof(Stack))
@ then
@ void* asm_stack_memory1 = asm_balloc(max_nodes * sizeof(Node) + sizeof(Stack))
@ use asm_balloc_free once: asm_balloc_free(asm_stack_memory)


@ R0 Bump Address
asm_balloc_free:
	LDR R1, =heap_head
	STR R2, [R1]
	STR R0, [R1]

done:
	BX LR
