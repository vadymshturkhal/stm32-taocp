#ifdef TRACE
#include "trace.hpp"

/*
 * The line, formatted by hand: no format string parsed for its types at run time, no
 * locale, no printf engine linked. The digits are written here, not with std::to_chars:
 * libstdc++'s to_chars (GCC 14.3) copies its 201-byte digit table onto the stack for
 * every number >= 10, because the table is a non-static constexpr local. On the G431 at
 * -O3, -flto on the C and the C++: ~1 430 cycles a line, against ~3 740 with to_chars
 * and ~10 400 with snprintf (same output, byte for byte).
 */

namespace {

/* Each one writes at p and returns the new end, or nullptr if it doesn't fit before end */

char* put(char* p, char* end, const char* s) noexcept
{
    for (; *s; ++s)
    {
        if (p == end) return nullptr;
        *p++ = *s;
    }
    return p;
}

/* The number of v's decimal digits */
std::size_t digit_count(std::uint32_t v) noexcept
{
    std::size_t n = 1;
    for (; v >= 10; v /= 10) ++n;
    return n;
}

/* v's decimal digits */
char* put(char* p, char* end, std::uint32_t v) noexcept
{
    const std::size_t n = digit_count(v);
    if (static_cast<std::size_t>(end - p) < n) return nullptr;
    char* q = p + n;
    do                                                    // last digit first
    {
        *--q = static_cast<char>('0' + v % 10);
        v /= 10;
    } while (v != 0);
    return p + n;
}

/* v with '0's in front up to width: printf's %04u */
char* put_zero_padded(char* p, char* end, std::uint32_t v, std::size_t width) noexcept
{
    for (std::size_t i = digit_count(v); i < width; ++i)
    {
        if (p == end) return nullptr;
        *p++ = '0';
    }
    return put(p, end, v);
}

/* Spaces after the field that began at start, up to width: printf's %-Nu, %-Ns */
char* pad(char* p, char* end, const char* start, std::size_t width) noexcept
{
    if (p == nullptr) return nullptr;
    while (static_cast<std::size_t>(p - start) < width)
    {
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
std::size_t format_trace(const TraceRecord& r, char* out, std::size_t size) noexcept
{
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
    for (const char* f = r.fmt; *f; ++f)
    {
        if (f[0] == '{' && f[1] == '}')
        {            // a number's place
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
