#include <stdint.h>
#include "stm32g4xx.h"
#include "stack_guard.h"

extern uint32_t _estack[];
extern uint8_t _Min_Stack_Size;   // 0x400 default
extern uint8_t _Stack_Guard_Size; // Address of the guard size
volatile uint32_t is_stack_guard_on = 0;
volatile StackGuardFault stack_guard_fault;

/* Guard starts at 0x20007C00 for 0x400 _Min_Stack_Size */
static uint32_t* stack_bottom(void)
{
    return (uint32_t*)((uintptr_t)_estack - (uintptr_t)&_Min_Stack_Size);
}

static uint32_t guard_bytes(void)
{
    return (uint32_t)(uintptr_t)&_Stack_Guard_Size;
}

void stack_guard_init(void)
{
    uint32_t* const bottom = stack_bottom();

    if (((uintptr_t)bottom & (guard_bytes() - 1u)) != 0u) return;

    uint32_t* p = bottom + guard_bytes() / 4u;
    uint32_t* const end = (uint32_t*)__get_MSP() - 16;

    // Paint a stack
    while (p < end) *p++ = STACK_PAINT;

    // Guard on
    MPU->RNR = 0u;                  // Region Number Register
    MPU->RBAR = (uint32_t)bottom;   // Region Base Addres Register

    // Region Attribute and Size Register
    MPU->RASR = MPU_RASR_XN_Msk     // XN = Execute Never
        | (0u << MPU_RASR_AP_Pos)   // AP = Access Permission
        | ((30u - __CLZ(guard_bytes())) << MPU_RASR_SIZE_Pos)    // Size Field
        | MPU_RASR_ENABLE_Msk;

    // Control Register
    // MPU_CTRL_PRIVDEFENA_Msk = Privileged Default map Enable
    MPU->CTRL = MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;
    __DSB();
    __ISB();

    is_stack_guard_on = 1;
}

/* 
 * How deep did the stack ever go since stack_guard_init()?
 * Can be compared with stack_guard_size() to get the margin.
 * Can be used for early warning, when used > 90% of the size.
 */
uint32_t stack_guard_depth(void)
{
    const uint32_t* p = (const uint32_t*)((uintptr_t)stack_bottom() + guard_bytes());
    while (p < _estack && *p == STACK_PAINT) ++p;
    return (uint32_t)((uintptr_t)_estack - (uintptr_t)p);
}

uint32_t stack_guard_size(void)
{
    return (uint32_t)(uintptr_t)&_Min_Stack_Size - guard_bytes();   /* 1024 - 32 = 992 */
}


/* The C part: runs on the fresh stack, old_msp = the stacked frame */
__attribute__((used, noreturn))
void stack_guard_record(const uint32_t* old_msp)
{
    const uint32_t cfsr = SCB->CFSR;
    stack_guard_fault.cfsr  = cfsr;
    stack_guard_fault.hfsr  = SCB->HFSR;
    stack_guard_fault.mmfar = SCB->MMFAR;
    stack_guard_fault.bfar  = SCB->BFAR;
    stack_guard_fault.sp    = (uint32_t)old_msp;

    /*The frame was stacked */
    if ((cfsr & (SCB_CFSR_MSTKERR_Msk | SCB_CFSR_STKERR_Msk)) == 0u)
    {
        stack_guard_fault.lr = old_msp[5];
        stack_guard_fault.pc = old_msp[6];
    }

    ++stack_guard_fault.count;
    
    /* Stop only with a debugger */
    if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) __BKPT(0);

    for(;;) {}
}

__attribute__((naked))  /* Tells GCC and Clang to compile this function without using PUSH/POP */
void stack_guard_hard_fault(void)
{
    __asm volatile(
        "mrs r0, msp            \n"   /* 1. the old SP: where the core stacked the frame */
        "ldr r1, =_estack       \n"   /* 2. the top of RAM */
        "msr msp, r1            \n"   /* 3. a fresh, empty stack */
        "b   stack_guard_record \n"   /* 4. into C, with the old SP as the argument (r0) */
        ".ltorg                 \n"); /* 5. the literal pool for "=_estack" */
}

#ifndef USE_HAL_DRIVER
/* Replaces the startup file's weak HardFault_Handler */
void HardFault_Handler(void) __attribute__((alias("stack_guard_hard_fault")));
#endif
