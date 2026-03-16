from random import shuffle


def j_inverse_in_place(permutation):
    # J1 (Negate all)
    for i in range(len(permutation)):
        permutation[i] = -permutation[i]

    m = len(permutation) - 1

    while m > 0:
       
        # J2 (Initialize)
        j = m

        # J3 (Find negative entry)
        # Set i = permutation[j], if i > 0: j = i and repeat
        i = permutation[j]
        while i > 0:
            j = i
            i = permutation[j]

        # J4 (Invert)
        # Set permutation[j] = permutation[-i], permutation[-i] = m
        permutation[j] = permutation[-i]
        permutation[-i] = m

        # J5 (Loop on m)
        # Decrease m by 1; if m > 0, go back to J2, terminate otherwise
        m -= 1


if __name__ == "__main__":
    # [0, 3, 2, 6, 5, 4, 1]
    permutation = [0, 6, 2, 1, 5, 4, 3]
    print(f"Original {permutation = }")
    j_inverse_in_place(permutation)
    print(f"Inversed {permutation = }")
    print()

    # random permutation
    permutation_length = 8
    permutation = [_ for _ in range(1, permutation_length + 1)]
    shuffle(permutation)
    permutation.insert(0, 0)
    print(f"Original {permutation = }")
    j_inverse_in_place(permutation)
    print(f"Inversed {permutation = }")
    