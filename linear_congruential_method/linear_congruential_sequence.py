class LinearCongruentialMethod:
    """
    0 < m
    0 <= a < m
    0 <= c < m
    0 <= X_0 < m
    """
    def __init__(self, m=10, a=7, c=7, X_0=7):
        self.m = m
        self.a = a
        self.c = c
        self.X = X_0

    def next_number(self):
        X = self.X
        self.X = (self.a * self.X + self.c) % self.m
        return X

if __name__ == "__main__":
    LCM = LinearCongruentialMethod()
    for _ in range(10):
        print(LCM.next_number())
