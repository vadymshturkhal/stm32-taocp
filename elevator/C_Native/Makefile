CC = gcc
CFLAGS = -std=c11 -Wall -O3 -flto

SRCS = storage_pool.c elevator_list.c elevator_settings.c values.c trace.c elevator.c users.c simulation.c main.c

all: elevator_sim elevator_sim_notrace

# Default target: traced, matches the ASM/STM32 ports' -DTRACE build.
elevator_sim: $(SRCS)
	$(CC) $(CFLAGS) -DTRACE -o $@ $(SRCS)

# TRACE undefined: trace() compiles to an empty function, matching the ASM/STM32 ports' no-TRACE build.
elevator_sim_notrace: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

clean:
	rm -f elevator_sim elevator_sim_notrace *.o

.PHONY: all clean
