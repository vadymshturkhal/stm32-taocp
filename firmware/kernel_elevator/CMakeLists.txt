# The C++ elevator on the Knuth kernel, NUCLEO-G431RB, CMSIS only (no HAL, no CubeMX).
# setup.sh runs this; by hand:
#   cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake
#   cmake --build build
cmake_minimum_required(VERSION 3.20)
project(elevator_bare C CXX ASM)

# The kernel and the elevator: the stm32-taocp checkout this folder is in
get_filename_component(REPO "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
option(KNUTH_KERNEL_STATS "the WAIT list's counters (sortins, list and walk lengths)" OFF)

set(CMSIS    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cmsis)
set(KERNEL   ${REPO}/knuth_kernel)
set(ELEVATOR ${REPO}/elevator/cpp_embedded_kernel)

add_executable(elevator
    src/main.c
    src/clock.c
    src/board_main.c
    src/sbrk.c
    src/init_fini.c
    ${CMSIS}/device/system_stm32g4xx.c         # SystemInit: the FPU on
    ${CMSIS}/device/startup_stm32g431xx.s      # the vector table, Reset_Handler
    ${KERNEL}/port/stm32g4/kernel_port.cpp
    ${ELEVATOR}/elevator.cpp
    ${ELEVATOR}/elevator_list.cpp
    ${ELEVATOR}/elevator_simulation.cpp
    ${ELEVATOR}/users.cpp
    ${ELEVATOR}/trace.cpp                      # empty without TRACE
    ${ELEVATOR}/board/pendsv/start_cpp_elevator_simulation.cpp
)
set_target_properties(elevator PROPERTIES
    SUFFIX .elf
    C_STANDARD 11   C_EXTENSIONS ON                               # -std=gnu11
    CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF   # -std=c++20
)
target_include_directories(elevator SYSTEM PRIVATE    # third-party: no warnings from them
    ${CMSIS}/core ${CMSIS}/device
)
target_include_directories(elevator PRIVATE
    src
    ${KERNEL} ${KERNEL}/port/stm32g4
    ${ELEVATOR} ${ELEVATOR}/board/pendsv
)
target_compile_definitions(elevator PRIVATE
    STM32G431xx
    $<$<BOOL:${KNUTH_KERNEL_STATS}>:KNUTH_KERNEL_STATS>
)
# The flags of the CubeIDE project the reference numbers come from: -O3 everywhere,
# -flto and -DNDEBUG on the C++ only
target_compile_options(elevator PRIVATE
    $<$<COMPILE_LANGUAGE:C,CXX>:-O3 -g3 -Wall -ffunction-sections -fdata-sections>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti -fno-use-cxa-atexit -flto -DNDEBUG>
)
target_link_options(elevator PRIVATE
    -T${CMAKE_CURRENT_SOURCE_DIR}/stm32g431rb.ld
    -nostartfiles                  # Reset_Handler is the entry: no crt0 (see init_fini.c)
    -Wl,--gc-sections -Wl,-Map=$<TARGET_FILE_DIR:elevator>/elevator.map
    -O3 -flto -static
)
# binutils >= 2.39 warns that the flash segment is writable and executable: it holds
# .init_array/.fini_array, writable by nature. Harmless here (flash); older ld lacks the option
execute_process(COMMAND ${CMAKE_C_COMPILER} -Wl,--help OUTPUT_VARIABLE ld_help ERROR_QUIET)
if(ld_help MATCHES "no-warn-rwx-segments")
    target_link_options(elevator PRIVATE -Wl,--no-warn-rwx-segments)
endif()
add_custom_command(TARGET elevator POST_BUILD COMMAND ${ARM_SIZE} $<TARGET_FILE:elevator>)
