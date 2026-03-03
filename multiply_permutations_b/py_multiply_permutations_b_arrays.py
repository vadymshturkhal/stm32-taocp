"""
Input example: (acf)(bd)(abd)(ef)
Output: (acefb)

This algorithm accomplishes essentially the same result as Algorithm A.
Assume that the elements permuted are named [x_0, x_2..x_(n-1)].
We use an auxiliary table T[0], T[2],..., T[n-1];
upon termination of this algorithm, x_i goes to x_j under the input
permutation if and only if T[i] = j;

Algorithm B:
    B1 (Init)
        Set T[k] <- k for k in range n;
        Prepare to scan the input from right to left;

    B2  (Next element)
        Examine the next element of input right to left;
        If the input has been exhausted, the algorithm terminates;
        If the element is a ')': set Z = 0 and repeat step B2;
        If it is a '(', go to B4;
        In other case element is x_i for some i;
        Go to B3;

    B3 (Change T[i])
        Exchange Z, T[i];
        If T[i] = 0: j = i;
        Go to step B2;

    B4 (Change T[j])
        Set T[j] = Z (at this point j is the row that shows a ')' entry
            in the notation of Table 2, corresponding to the right 
            parenthesis that matches the left parenthesis just scanned)
        Return to B2;
"""

def find_untagged(original):
    for i in range(len(original)):
        if original[i] != 0:
            return i
        
    return -1

def get_multiplication_result(original: list, auxiliary_table: list) -> list:
    multiplication_result = []
    char_index = 0

    while True:
        # get untagged char index
        char_index = find_untagged(original)

        if char_index == -1:
            return multiplication_result

        # add left parenthesis
        multiplication_result.append('(')

        # get char
        char = original[char_index]
        while char_index != -1:
            # tag char
            original[char_index] = 0

            # add a char to multiplication_result
            multiplication_result.append(char)

            # set next char
            char = auxiliary_table[char_index]

            # find char index of the next char
            char_index = -1
            for i in range(len(original)):
                if char == original[i]:
                    char_index = i
                    break
            
        # add right parenthesis
        multiplication_result.append(')')


def multiply_permutations_b_arrays(permutation):
    # B1
    # auxiliary_table = create_identity_permutation(permutation)

    original = []
    auxiliary_table = []

    i = 0
    j = 0
    Z = 0

    # B2
    for index in range(len(permutation) - 1, -1, -1):
        char = permutation[index]

        # If the element is a ')': set Z = 0 and repeat step B2;
        if char == ')':
            Z = 0
            continue

        # B4
        if char == '(':
            # Set T[j] = Z 
            auxiliary_table[j] = Z

            # Return to B2;
            continue

        if char not in original:
            # auxiliary_table[char] = char
            original.append(char)
            auxiliary_table.append(char)
            i = len(auxiliary_table) - 1

        # B3
        # Exchange Z, T[i];
        # Z, auxiliary_table[char] = auxiliary_table[char], Z
        i = original.index(char)
        Z, auxiliary_table[i] = auxiliary_table[i], Z

        # If T[i] = 0: j = i;
        if auxiliary_table[i] == 0:
            j = i
        
        # Go to step B2;

    # Output
    multiply_permutation = get_multiplication_result(original, auxiliary_table)
    return multiply_permutation


if __name__ == "__main__":
    permutation = "(acf)(bd)(abd)(ef)"
    output = "(acefb)(d)"
    result = multiply_permutations_b_arrays(permutation)
    result = ''.join(result)
    print(result, output, result == output)

    permutation = "(acfg)(bcd)(aed)(fade)(bgfae)"
    output = "(adg)(ceb)(f)"
    result = multiply_permutations_b_arrays(permutation)
    result = ''.join(result)
    print(result, output, result == output)

