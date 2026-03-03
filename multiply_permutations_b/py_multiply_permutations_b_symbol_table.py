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

def multiply_permutations_b_symbol_table(permutation):
    # B1
    # auxiliary_table = create_identity_permutation(permutation)

    auxiliary_table = dict()

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

        if char not in auxiliary_table:
            auxiliary_table[char] = char

        # B3
        # Exchange Z, T[i];
        Z, auxiliary_table[char] = auxiliary_table[char], Z

        # If T[i] = 0: j = i;
        if auxiliary_table[char] == 0:
            j = char
        
        # Go to step B2;

    # Output

    # Is there a singleton cycle?
    if len(auxiliary_table) == 1:
        return "()"

    multiplication_result = []

    # Open a cycle
    for char in permutation:
        if char in ('(', ')'):
            continue

        if auxiliary_table[char] == 0:
            continue


        multiplication_result.append('(')

        while True:
            next_char = auxiliary_table[char]
            if next_char == 0:
                break
            
            multiplication_result.append(char)
            auxiliary_table[char] = 0
            char = next_char

        multiplication_result.append(')')

    return multiplication_result


if __name__ == "__main__":
    permutation = "(acf)(bd)(abd)(ef)"
    output = "(acefb)(d)"
    result = multiply_permutations_b_symbol_table(permutation)
    result = ''.join(result)
    print(result, output, result == output)

    permutation = "(acfg)(bcd)(aed)(fade)(bgfae)"
    output = "(adg)(ceb)(f)"
    result = multiply_permutations_b_symbol_table(permutation)
    result = ''.join(result)
    print(result, output, result == output)
