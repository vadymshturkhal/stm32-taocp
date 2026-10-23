#include "kernel_port.hpp"
#include "stm32g4xx_hal.h"  // Only for the RCC macro

volatile uint32_t pendsv_count = 0;  // global, visible in Live Expressions

extern "C" void kernel_port_init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();  // macro enables the clock and then reads the register back
    TIM2->PSC = 170 - 1;  // the prescaler: divides the input clock, so 170 MHz becomes 1 tick per µs
    TIM2->ARR = 0xFFFFFFFF;  // auto-reload: the value at which it wraps. 0xFFFFFFFF uses all 32 bits
    TIM2->EGR = TIM_EGR_UG;  // control: force an update
    TIM2->CR1 = TIM_CR1_CEN;  // control: start

    NVIC_SetPriority(PendSV_IRQn, (1u << __NVIC_PRIO_BITS) - 1);  // lowest: 15
}

extern "C" void kernel_pendsv(void) {
    pendsv_count = pendsv_count + 1;
}
