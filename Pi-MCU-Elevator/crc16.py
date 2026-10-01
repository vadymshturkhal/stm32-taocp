def crc16(data: bytes) -> int:
    # 1. Start with crc = 0xFFFF
    crc = 0xFFFF

    # 2. For each byte, XOR it into the top 8 bits of crc
    for byte in data:
        byte<<=8
        crc^= byte

        # 3. Repeat 8 times:
        for _ in range(8):
            # look at the top bit
            top_bit = crc >> 15

            # shift crc left by 1
            crc <<= 1

            # and if the top is 1: XOR the polynomial
            if top_bit:
                crc ^= 0x1021

            # trim to 16 bits long
            crc &= 0xFFFF

    return crc


if __name__ == "__main__":
    res = crc16("A".encode("ascii"))
    print(res)
    print(0xB915)
