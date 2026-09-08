class LinearCongruentialMethod:
    """
    0 < m
    0 <= a < m
    0 <= c < m
    0 <= X_0 < m
    b = a - 1: a >= 2, b >= 1
    
    e = 32    # e-bit computer
    w = 2**e  # word size
    the result given modulo w
    """

    def __init__(self, a=7, c=0, X_0=7):
        self.a = a
        self.c = c
        self.X = X_0
        self.e = 8
        self.w = 2**self.e
        self.m = self.w + 1
    
    # def next_number(self):
    #     """
    #     Computes (aX + c) mod w.

    #     MIX instructions:
    #     LDA A
    #     MUL X
    #     SLAX 5
    #     ADD C
    #     """

    #     X = self.X
    #     mask = self.w - 1
    #     product = self.a * self.X
    #     lower_half_result = product & mask
    #     self.X = (lower_half_result + self.c) & mask
    #     return X

    def next_number(self):
        """
        Computes (aX) mod m, where m = w + 1
        """

        X = self.X
        self.X = (self.a * self.X) % (self.w + 1)
        return X

    def next_number_clever(self):
        """
        Computes (aX) mod m, where m = w + 1

        MIX instructions:
        LDAN X      # load negative X to rA
        MUL A       # multiply rA by A, upper bytes are placed in rA, lower in rX
        STX TEMP
        SUB TEMP
        JANN *+3
        INCA 2
        ADD =w-l=
        """

        X = self.X
        product = X * self.a
        q, r = divmod(product, self.w)  # MUL: high word q (rA), low word r (rX)
        A = r - q                        # STX TEMP; SUB TEMP

        if A < 0:
            A += 2          # INCA 2
            A += self.w - 1  # ADD =w-l=

        self.X = A
        return X

def check_sequence(sequence):
    mid = len(sequence) // 2
    is_all_equal = True
    for i in range(mid):
        if sequence[i] != sequence[mid + i]:
            is_all_equal = False

    return is_all_equal

def retrieve_period(sequence):
    return sequence[:len(sequence)//2]


if __name__ == "__main__":
    # m = 7
    # a = 12
    # c = 71
    # X_0 = 12

    # m = 10
    # a = 7
    # c = 7
    # X_0 = 7
    # LCM = LinearCongruentialMethod(m, a, c, X_0)

    LCM = LinearCongruentialMethod()
    sequence = []

    first_term = LCM.next_number()
    print(f"{first_term = }")
    for _ in range(1000):
        next_number = LCM.next_number()
        sequence.append(next_number)

        if len(sequence) < 4:
            continue

        is_found_periond = check_sequence(sequence)

        if is_found_periond:
            # print(sequence)
            period = retrieve_period(sequence)
            print(f"{period = }")
            print(f"{len(period) = }")
            break

    print()
    sequence.clear()
    for _ in range(1000):
        next_number = LCM.next_number_clever()
        sequence.append(next_number)

        if len(sequence) < 4:
            continue

        is_found_periond = check_sequence(sequence)

        if is_found_periond:
            # print(sequence)
            period = retrieve_period(sequence)
            print(f"{period = }")
            print(f"{len(period) = }")
            break
