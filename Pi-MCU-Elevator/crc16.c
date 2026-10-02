#include "crc16.h"

uint16_t crc16(const uint8_t* data, size_t len) {
    // 1. Start with crc = 0xFFFF
    uint16_t crc = 0xFFFF;

    // 2. For each byte, XOR it into the top 8 bits of crc
    for (size_t i = 0; i < len; ++i) {
        // uint16_t byte = data[i];
        // byte <<= 8;
        // crc ^= byte;

        crc ^= (uint16_t)(data[i] << 8);

        // 3. Repeat 8 times
        for (uint32_t bit = 0; bit < 8; ++bit) {
            // look at the top bit
            uint16_t top_bit = crc >> 15;
            
            // shift crc left by 1
            crc <<= 1;

            // and if the top is 1: XOR the polynomial
            if (top_bit) crc ^= 0x1021;

            // doesn't need a trim due to the overflow
        }
    }

    return crc;
}
