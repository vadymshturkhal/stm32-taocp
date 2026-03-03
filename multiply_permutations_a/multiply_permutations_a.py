# Input example: (acf)(bd)(abd)(ef)
# Output: (acefb)


# Algorithm A

# A1
# Tag all left parentheses, and replace each right parenthesis
# by a tagged copy of the element that follows its matching left parenthesis

# A2
# If all chars are tagged: return output string;
# Searching from left to right, find the first untagged element of the input.
# Set START equal to it;
# Add a left parenthesis to the output;
# Add the element to the output;

# A3
# CURRENT = next element;

# A4
# loop to the right until the end of the string;
# if found CURRENT: go to A3

# A5 CURRENT == START?
# If CURRENT != START:
# add CURRENT to the output;
# go to A4;

# A6 Complete cycle has been found
# Add a right parenthesis to the output;
# go to step A2;

# Overall:
# permutation;
# multiplication_result;
# tag/untag array or hash table;
# START;
# CURRENT;

# A1
def first_pass(permutation, tagged) -> list:
    """
    Tag all left parentheses, 
    and replace each right parenthesis by a tagged copy 
    of the element that follows its matching left parenthesis.
    """

    parsed_permutation = []

    cycle_begin = None

    for i in range(len(permutation)):
        char = permutation[i]

        if char == '(':
            tagged[i] = 1
            cycle_begin = permutation[i+1]
        
        if char == ')':
            tagged[i] = 1
            char = cycle_begin

        parsed_permutation.append(char)


    return parsed_permutation

# A2
def search_untagged(parsed_permutation, tagged) -> int:
    """
    Searching from left to right, find the first untagged element of the input
    and return its index;
    """

    start = None
    for i in range(len(parsed_permutation)):
        if not tagged[i]:
            return i

    return start

def find_cycle(parsed_permutation, tagged, multiplication_result, start_index):
    start = parsed_permutation[start_index]

    # A3
    current = parsed_permutation[start_index]

    while True:
        # A4
        i = start_index
        while i < len(parsed_permutation) - 1:
            char = parsed_permutation[i]

            if char == current:
                tagged[i] = 1

                # A3
                current = parsed_permutation[i+1]
                i += 1
            
            i += 1
        
        # A6
        if current == start:
            multiplication_result.append(')')
            return

        # A5
        multiplication_result.append(current)


def multiply_permutations_a(permutation):
    tagged = [0] * len(permutation)
    multiplication_result = []

    # A1
    parsed_permutation = first_pass(permutation, tagged)

    while True:
        # A2
        start_index = search_untagged(parsed_permutation, tagged)

        if start_index is None:
            return multiplication_result

        start = parsed_permutation[start_index]
        multiplication_result.append('(')
        multiplication_result.append(start)
        tagged[start_index] = 1

        # A3, A4, A5, A6
        find_cycle(parsed_permutation, tagged, multiplication_result, start_index)


if __name__ == "__main__":
    permutation = "(acf)(bd)(abd)(ef)"
    output = "(acefb)(d)"
    result = multiply_permutations_a(permutation)
    result = ''.join(result)
    print(result, output, result == output)

    permutation = "(acfg)(bcd)(aed)(fade)(bgfae)"
    output = "(adg)(ceb)(f)"
    result = multiply_permutations_a(permutation)
    result = ''.join(result)
    print(result, output, result == output)
