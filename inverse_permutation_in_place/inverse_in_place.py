from random import shuffle


def inverse_in_place(permutation):
    # I1 (Initialize)
    m = len(permutation) - 1
    j = -1

    while m > 0:       
        # I2 (Next element)
        i = permutation[m]

        # Go to I5 (the element has already been processed)
        if i < 0: 
            # I5 (store final value)
            # X[m] = -i (Originally X[i] was equal to m)
            permutation[m] = -i 

            # I6 (loop on m)
            # Decrease m by 1;
            # if m > 0: go to I2 else done
            m -= 1
            continue

        # I3 (Invert one): 
        # At this point j < 0 and i = X[m];
        # if m is not the largest element of its cycle,
        # the original permutation had X[-j] = m);
        # set X[m] = j, j = -m, m = i, i = X[m];
        while i > 0:
            permutation[m] = j
            j = -m
            m = i
            i = permutation[m]

            # I4 (End of cycle?) 
            # If i > 0, go back to I3 (the cycle has not ended);
            # otherwise i = j (In the latter case, the original permutation had X[-j] = m and m is the largest in its cycle)

        i = j

        # I5 (store final value)
        # X[m] = -i (Originally X[i] was equal to m)
        permutation[m] = -i 

        # I6 (loop on m)
        # Decrease m by 1;
        # if m > 0: go to I2 else done
        m -= 1


if __name__ == "__main__":
    # [0, 3, 2, 6, 5, 4, 1]
    permutation = [0, 6, 2, 1, 5, 4, 3]
    print(f"Original {permutation = }")
    inverse_in_place(permutation)
    print(f"Inversed {permutation = }")
    print()

    # random permutation
    permutation_length = 6
    permutation = [_ for _ in range(1, permutation_length + 1)]
    shuffle(permutation)
    permutation.insert(0, 0)
    print(f"Original {permutation = }")
    inverse_in_place(permutation)
    print(f"Inversed {permutation = }")
    