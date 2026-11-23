#ifdef TRACE
#include <charconv>
#include <cstring>
#include "trace.hpp"

/*
 * The line, formatted with std::to_chars: no format string parsed for its types at run
 * time, no locale, no printf engine linked. On the G431 at -O3 -flto: ~3 700 cycles a
 * line, against ~10 400 with the snprintf version it replaced (same output, byte for byte).
 */

namespace {

// Each one writes at p and returns the new end, or nullptr if it doesn't fit before end

char* put(char* p, char* end, const char* s) noexcept {
    for (; *s; ++s) {
        if (p == end) return nullptr;
        *p++ = *s;
    }
    return p;
}

char* put(char* p, char* end, std::uint32_t v) noexcept {
    const auto r = std::to_chars(p, end, v);
    return r.ec == std::errc{} ? r.ptr : nullptr;
}

// v with '0's in front up to width: printf's %04u
char* put_zero_padded(char* p, char* end, std::uint32_t v, std::size_t width) noexcept {
    char digits[10];                                   // 4294967295: 10 digits
    const auto r = std::to_chars(digits, digits + sizeof digits, v);
    const auto n = static_cast<std::size_t>(r.ptr - digits);
    for (std::size_t i = n; i < width; ++i) {
        if (p == end) return nullptr;
        *p++ = '0';
    }
    if (static_cast<std::size_t>(end - p) < n) return nullptr;
    std::memcpy(p, digits, n);
    return p + n;
}

// spaces after the field that began at start, up to width: printf's %-Nu, %-Ns
char* pad(char* p, char* end, const char* start, std::size_t width) noexcept {
    if (p == nullptr) return nullptr;
    while (static_cast<std::size_t>(p - start) < width) {
        if (p == end) return nullptr;
        *p++ = ' ';
    }
    return p;
}

}  // namespace

/*
 * "TIME   STATE FLOOR D1 D2 D3  step action\n", the columns as the snprintf version's
 * "%04u   %-5s %-4u %-2u %-2u %-2u  %s ". The action is the record's format with each
 * "{}" replaced by the next number; TraceFormat checked at compile time that their counts
 * match. Returns the bytes to send, or 0 if the line doesn't fit (nothing to send).
 */
std::size_t format_trace(const TraceRecord& r, char* out, std::size_t size) noexcept {
    if (size == 0) return 0;
    char* const end = out + size - 1;   // the line, '\n' included, fits in size - 1, as with snprintf's NUL
    char* p = out;
    const char* field;

    if (!(p = put_zero_padded(p, end, r.time, 4))) return 0;
    if (!(p = put(p, end, "   "))) return 0;
    field = p; if (!(p = pad(put(p, end, r.state), end, field, 5))) return 0;
    if (!(p = put(p, end, " "))) return 0;
    field = p; if (!(p = pad(put(p, end, std::uint32_t{r.floor}), end, field, 4))) return 0;
    if (!(p = put(p, end, " "))) return 0;
    field = p; if (!(p = pad(put(p, end, std::uint32_t{r.d1}), end, field, 2))) return 0;
    if (!(p = put(p, end, " "))) return 0;
    field = p; if (!(p = pad(put(p, end, std::uint32_t{r.d2}), end, field, 2))) return 0;
    if (!(p = put(p, end, " "))) return 0;
    field = p; if (!(p = pad(put(p, end, std::uint32_t{r.d3}), end, field, 2))) return 0;
    if (!(p = put(p, end, "  "))) return 0;
    if (!(p = put(p, end, r.step))) return 0;
    if (!(p = put(p, end, " "))) return 0;

    std::size_t arg = 0;
    for (const char* f = r.fmt; *f; ++f) {
        if (f[0] == '{' && f[1] == '}') {            // a number's place
            if (arg == 4 || !(p = put(p, end, r.args[arg++]))) return 0;
            ++f;
            continue;
        }
        if (p == end) return 0;
        *p++ = *f;
    }

    if (p == end) return 0;
    *p++ = '\n';
    return static_cast<std::size_t>(p - out);
}
#endif  // TRACE
