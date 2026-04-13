class Node:
        def __init__(self, succ=None, next_=None):
            self.succ = succ
            self.next = next_

        def __repr__(self):
            succ_val = self.succ if self.succ else "None"
            next_val = self.next if self.next else "None"
            return f"Node(succ={succ_val}, next={next_val})"


def algorithm_t(n, input_pairs):
    # T1 (Initialize)
    COUNT = [0 for _ in range(n+1)]
    # QLINK = [0 for _ in range(n+1)]
    QLINK = COUNT
    TOP = [None for _ in range(n+1)]
    N = n
    sorted_sequence = []

    # T2 (Next relation)
    for pair in input_pairs:
        j, k = pair
        
		# T3 (Record the relation)
        COUNT[k] += 1
        P = Node(succ=k, next_=TOP[j])
        TOP[j] = P

	# T4 (At this point we have completed the input phase)
    R = 0
    QLINK[0] = 0
    for k in range(1, n+1):
        if COUNT[k] == 0:
            QLINK[R] = k
            R = k

    F = QLINK[0]

    while N != 0:
        # T5 (Output front of queue)
        # print(f'{F=}')
        sorted_sequence.append(F)
        if F == 0:
            # Go to T8
            break

        N = N - 1
        P = TOP[F]

        # T6 (Erase relations)
        while P is not None:
            # print(P)
            COUNT[P.succ] -= 1

            if COUNT[P.succ] == 0:
                QLINK[R] = P.succ
                R = P.succ
            
            P = P.next
        
        # T7 (Remove from Queue)
        F = QLINK[F]

        # Go back to T5

    # T8 (End of process)
    return sorted_sequence


if __name__ == "__main__":
    # [1, 9, 3, 2, 7, 4, 5, 8, 6]
    n = 9
    input_pairs = [
        (9, 2),
        (3, 7),
        (7, 5),
        (5, 8),
        (8, 6),
        (4, 6),
        (1, 3),
        (7, 4),
        (9, 5),
        (2, 8),
    ]
    print(algorithm_t(n, input_pairs))

    # [8, 7, 3, 2, 1, 6, 5, 4]
    n = 8
    input_pairs = [
        (8, 3),
        (1, 4),
        (7, 2),
        (3, 1),
        (2, 6),
        (8, 7),
        (1, 5),
        (3, 2),
        (7, 6),
        (2, 5),
    ]
    print(algorithm_t(n, input_pairs))

    # [4, 6, 1, 5, 2, 3]
    n = 6
    input_pairs = [
        (4, 5),
        (1, 3),
        (6, 2),
        (5, 3),
        (4, 1),
        (2, 3),
        (6, 5)
    ]
    print(algorithm_t(n, input_pairs))
