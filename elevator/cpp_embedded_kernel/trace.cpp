#ifdef TRACE
#include <cinttypes>
#include <cstdio>
#include "trace.hpp"

/* Return the number of bytes to send. */
std::size_t format_trace(const TraceRecord& record, char* out, std::size_t size) noexcept {
    int n = std::snprintf(out, size, "%04" PRIu32 "   %-5s %-4u %-2u %-2u %-2u  %s ",
            record.time, record.state, record.floor, record.d1, record.d2, record.d3, record.step);

    if (n < 0 || static_cast<std::size_t>(n) >= size) return 0;
    
    int m = std::snprintf(out + n, size - static_cast<std::size_t>(n), 
            record.fmt, record.args[0], record.args[1], record.args[2], record.args[3]);

    if (m < 0 || static_cast<std::size_t>(n + m + 1) >= size) return 0;
    out[n + m] = '\n';
    return static_cast<std::size_t>(n + m + 1);
}
#endif
