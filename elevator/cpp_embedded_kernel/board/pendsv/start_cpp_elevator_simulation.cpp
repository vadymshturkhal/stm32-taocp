// start_cpp_elevator_simulation.cpp - one complete run of the C++ elevator, called from C
//
// Like the C start_elevator_simulation(): every call starts from a fresh state
// (a new simulation with its users' pool, VALUES from seed 1) and runs until the
// elevator is finished, so a cold and a warm run do the same work.
//
// The kernel is the port's shared one (kernel_port.hpp): it outlives the
// elevator, so the elevator must leave no nodes in it when it is destroyed.
// finished() says when: every user gone and the elevator dormant at E1, so
// none of its nodes is in the list.
// The simulation lives in static storage, not on the stack: it is a few hundred
// bytes and the linker script reserves only 1 KB of stack (_Min_Stack_Size).
// std::optional builds it in place for each run and destroys it after it
// (the destructor of the users' pool gives its memory back, as C's free() does).
#include <cassert>
#include <cstdint>
#include <optional>

#include "kernel_port.hpp"
#include "elevator_simulation.hpp"
#include "start_cpp_elevator_simulation.h"
#include "trace.hpp"

#ifdef TRACE
#include "../uart_tx.h"
#endif

static std::optional<ElevatorSimulation> simulation;

// Construction (the users' pool: malloc and every node built) is most of the
// setup, so it is split from start: read "now" after it, and USER1 is not late
extern "C" void cpp_elevator_create(void) {
    simulation.emplace(kernel);
}

extern "C" void cpp_elevator_start(std::uint32_t max_users, std::uint32_t now_ms) {
    if (!simulation) cpp_elevator_create();
    simulation->start(max_users, now_ms);   // Knuth's TIME 0 = now: USER1 is due at once
}

// main polls it; PendSV sets it (check_done) at the step that ends the run. The
// atomic is re-read on every call, and its acquire makes the checksum and the
// counters written before it visible to main.
extern "C" int cpp_elevator_finished(void) {
    return simulation && simulation->done.load(std::memory_order_acquire) ? 1 : 0;
}

// Both for release builds, 0 in a correct run. Read them before stop.
extern "C" std::uint32_t cpp_elevator_checksum(void) {
    return simulation ? simulation->checksum : 0;
}

extern "C" std::uint32_t cpp_elevator_pool_refused(void) {
    return simulation ? simulation->users.pool_refused() : 0;
}

// main's side of the trace: PendSV's steps write records, main formats them here, where
// PendSV can preempt it, and queues the lines. Never waits: if every TX buffer is busy,
// the record stays the oldest and the next call tries again.
extern "C" void cpp_elevator_trace_poll(void) {
#ifdef TRACE
    while (TraceRecord* r = trace_circle.start_operation()) {
        char* line = UART_TX_Begin();
        if (line == nullptr) {
            trace_circle.cancel_operation();
            return;
        }

        UART_TX_Commit(format_trace(*r, line, UART_TX_LINE_SIZE));
        trace_circle.operation_is_completed();
    }
#endif
}

// 1 while trace records wait for cpp_elevator_trace_poll()
extern "C" int cpp_elevator_trace_pending(void) {
#ifdef TRACE
    return trace_circle.red_buffers_quantity() != 0 ? 1 : 0;
#else
    return 0;
#endif
}

// Records lost because the trace circle was full: 0 expected
extern "C" std::uint32_t cpp_elevator_trace_dropped(void) {
#ifdef TRACE
    return trace_dropped;
#else
    return 0;
#endif
}

extern "C" void cpp_elevator_stop(void) {
    // Safe once finished(): none of the elevator's nodes is in the shared kernel.
    // TODO: stopping in the middle of a run needs the elevator to cancel its own nodes.
    assert(!simulation || simulation->finished());
    simulation.reset();
}
