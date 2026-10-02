## CRC16

Every protocol line carries a CRC, so the receiver can tell a line damaged on
the UART (a flipped bit, a lost byte) from a good one and drop it instead of
acting on it.

### Parameters

| Parameter    | Value                       |
|--------------|-----------------------------|
| Name         | CRC-16/CCITT-FALSE          |
| Width        | 16 bits                     |
| Polynomial   | `0x1021` (x¹⁶ + x¹² + x⁵ + 1) |
| Initial value| `0xFFFF`                    |
| Bit order    | MSB first, no reflection    |
| Final XOR    | none                        |
| Check value  | `"123456789"` → `0x29B1`    |

Any implementation with these parameters matches ours. In Python the
built-in one is `binascii.crc_hqx(data, 0xFFFF)`.

### Algorithm

1. Start with `crc = 0xFFFF`.
2. For each byte, XOR it into the top 8 bits of `crc`.
3. Repeat 8 times: remember the top bit, shift `crc` left by 1, and if the
   top bit was 1, XOR in `0x1021`. Keep `crc` 16 bits wide.
4. After the last byte, `crc` is the result.

### What it detects

- every 1-bit and 2-bit error
- every odd number of flipped bits
- every burst of errors up to 16 bits long

It is not security: anyone can compute a matching CRC for a changed line.

### Files

| File             | Contents                                                        |
|------------------|-----------------------------------------------------------------|
| `crc16.py`       | Python version, for the Pi                                      |
| `crc16.h/.c`     | C version                                                       |
| `crc16.hpp`      | C++ version, header only and `constexpr`: `crc16(std::span<const std::uint8_t>)` for bytes, `crc16(std::string_view)` for text |
| `test_crc16.py`  | Python test: the table below, plus 1000 random inputs compared with `binascii` |
| `test_crc16.cpp` | C++ test: the table below with `static_assert` (checked while compiling) and at run time through the `span` version |

### Test values

| Input           | CRC    |
|-----------------|--------|
| `""` (empty)    | `FFFF` |
| `"A"`           | `B915` |
| `"123456789"`   | `29B1` |
| `"0"`           | `D7A3` |
| `"2 0 5 hello"` | `EBDC` |

### Running the tests

```
python3 test_crc16.py; echo $?

g++-14 -std=c++23 -Wall -Wextra -Wconversion test_crc16.cpp -o test_crc16_cpp && ./test_crc16_cpp; echo $?
```

Both exit with 0 when every test passes. If a `static_assert` fails, the C++
build itself stops, so a wrong CRC never produces a program. The C++ test
needs GCC 14 for `<print>`.
