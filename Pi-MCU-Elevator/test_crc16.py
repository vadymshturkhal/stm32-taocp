import binascii
import random
import sys
from crc16 import crc16


class ManualTestCases:
    def __init__(self):
        self.TEST_CASES = [    
            (b"", 0xFFFF),
            (b"A", 0xB915),
            (b"123456789", 0x29B1),
            (b"0", 0xD7A3),
            (b"2 0 5 hello", 0xEBDC),
        ]

    def run(self):
        for data, expected in self.TEST_CASES:
            got = crc16(data)
            if got != expected:
                print(f"Manual CRC16 is not correct: got {got:#06x} and correct is {expected:#06x} for {data.hex()}")
                return 1

        print("All manual tests passed, CRC16 is correct")
        return 0

class RandomTestCases:
    def __init__(self, n):
        self.n = n
        self.max_bytes_len = 100

    def run(self):
        random.seed(0)
        for _ in range(self.n):
            bytes_len = random.randint(0, self.max_bytes_len)
            random_data = random.randbytes(bytes_len)

            got = crc16(random_data)
            expected = binascii.crc_hqx(random_data, 0xFFFF)
            if got != expected:
                print(f"Random CRC16 is not correct: got {got:#06x} and correct is {expected:#06x} for {random_data.hex()}")
                return 1

        print("All random tests passed, CRC16 is correct")
        return 0


if __name__ == "__main__":
    manual_test = ManualTestCases()
    status = manual_test.run()
    if status != 0: sys.exit(1)

    n = 1000
    random_test = RandomTestCases(n)
    status = random_test.run()
    if status != 0: sys.exit(1)
