from storage_pool import STORAGE_POOL


class PolynomialNode:
    def __init__(self, COEFF=None, ABC=None, link=None):
        self.link = link
        self.COEFF = COEFF
        self.ABC = ABC

    def __str__(self):
        return f"{self.COEFF}, {self.ABC}"

class NodeInfo:
    def __init__(self, COEFF, sign, A, B, C):
        self.COEFF = COEFF
        self.sign = sign
        self.ABC = self.sign * int(f"{A:04d}{B:04d}{C:04d}")

class PolynomialCircularList:
    """
    Node class used by the storage_pool must have 'link' and 'info' attributes.
    """
    def __init__(self, storage_pool):
        self.storage_pool = storage_pool
        self.ptr = None

    def insert_left(self, Y:NodeInfo):
        P = self.storage_pool.pop()
        P.COEFF = Y.COEFF
        P.ABC = Y.ABC
        
        if self.ptr is None:
            P.link = P
            self.ptr = P
        else:
            P.link = self.ptr.link
            self.ptr.link = P
        
    def insert_right(self, Y):
        self.insert_left(Y)
        self.ptr = self.ptr.link

    def __str__(self):
        if self.ptr is None:
            return ""

        terms = []
        P = self.ptr.link

        is_first_term = True

        while P.ABC > 0:
            ABC = str(P.ABC)
            ABC_len = len(ABC)

            if ABC_len < 12:
                ABC = ABC.zfill(12)

            x_power = int(ABC[0:4])
            y_power = int(ABC[4:8])
            z_power = int(ABC[8:12])

            term_block = []
            if is_first_term:
                coeff = "" if P.COEFF == 1 else str(P.COEFF)
            else:
                if P.COEFF == 1:
                    coeff = f"+ "
                    # coeff = "" if P.COEFF == 1 else str(P.COEFF)
                elif P.COEFF < 0:
                    coeff = f"- {-P.COEFF}" if P.COEFF != -1 else "- "
                else:
                    coeff = "+ " if P.COEFF == 1 else f"+ {P.COEFF}"
                
            term_block.append(coeff)

            if x_power != 0:
                x_term = f"x**{x_power}" if x_power != 1 else "x"
                term_block.append(x_term)
            
            if y_power != 0:
                y_term = f"y**{y_power}" if y_power != 1 else "y"
                term_block.append(y_term)

            if z_power != 0:
                z_term = f"z**{z_power}" if z_power != 1 else "z"
                term_block.append(z_term)

            term = ''.join(term_block)
            terms.append(term)

            P = P.link
            is_first_term = False

        terms = ' '.join(terms)
        return terms

def fill_polynomial(polynomial:PolynomialCircularList, terms: tuple):
    if polynomial == None:
        return 1

    if len(terms) < 1:
        return 2

    for term in terms:
        node_info = NodeInfo(*term)
        polynomial.insert_right(node_info)

    return 0

def addition_of_polynomials(P: PolynomialNode, Q: PolynomialNode, storage_pool):
    # A1. [Initialize]
    P = P.link
    Q1 = Q
    Q = Q.link

    while True:
        # A2. [ABC(P):ABC(Q)]
        while P.ABC < Q.ABC:
            Q1 = Q
            Q = Q.link
            continue

        if P.ABC > Q.ABC:
            # A5. [Insert new term]
            # Q2 <= AVAIL
            Q2 = storage_pool.pop()
            Q2.COEFF = P.COEFF
            Q2.ABC = P.ABC
            Q2.link = Q
            Q1.link = Q2
            Q1 = Q2
            P = P.link
            
            # Go to step 2 
            continue

        # Go to A3
        # if P.ABC == Q.ABC:
        #     pass

        # A3. [Add coefficients]
        if P.ABC < 0:
            return 0
        
        Q.COEFF += P.COEFF
        if Q.COEFF != 0:
            # FIXME Go to A1
            P = P.link
            Q1 = Q
            Q = Q.link
    
            # Go to A2
            continue

        # Go to A4
        # if Q.COEFF == 0:
        #     pass

        # A4. [Delete zero term]
        Q2 = Q
        Q = Q.link
        Q1.link = Q

        # AVAIL <= Q2
        storage_pool.push(Q2)

        P = P.link
        
        # Go to A2
        continue

if __name__ == "__main__":
    polynomial_P_size = 4
    polynomial_Q_size = 4
    max_size = 2 + 4 * (polynomial_P_size + polynomial_Q_size)
    storage_pool = STORAGE_POOL(PolynomialNode, size=max_size)

    polynomial_P = PolynomialCircularList(storage_pool)
    polynomial_Q = PolynomialCircularList(storage_pool)

    P_terms = ( (1,1,1,0,0), (1,1,0,1,0), (1,1,1,0,1), (0,-1,0,0,1) )  # x + y + xz
    Q_terms = ( (1,1,2,0,0), (-2,1,0,1,0), (-1,1,1,0,1), (0,-1,0,0,1) )  # x**2 - 2y - xz

    fill_polynomial(polynomial_P, P_terms)
    fill_polynomial(polynomial_Q, Q_terms)
    print(polynomial_P)
    print(polynomial_Q)

    addition_of_polynomials(polynomial_P.ptr, polynomial_Q.ptr, storage_pool)

    Q = polynomial_Q.ptr.link
    print(polynomial_Q)
    # while Q.ABC > 0:
    #     print(Q)
    #     Q = Q.link

    print()

    polynomial_P_size = 3
    polynomial_Q_size = 3
    max_size = 2 + 4 * (polynomial_P_size + polynomial_Q_size)
    storage_pool = STORAGE_POOL(PolynomialNode, size=max_size)

    polynomial_P = PolynomialCircularList(storage_pool)
    polynomial_Q = PolynomialCircularList(storage_pool)

    P_terms = ( (1,1,3,0,0), (1,1,0,0,2), (0,-1,0,0,1) )  # x**3 + z**2
    Q_terms = ( (1,1,0,3,0), (1,1,0,0,2), (0,-1,0,0,1) )  # y**3 + z**2

    fill_polynomial(polynomial_P, P_terms)
    fill_polynomial(polynomial_Q, Q_terms)
    print(polynomial_P)
    print(polynomial_Q)

    addition_of_polynomials(polynomial_P.ptr, polynomial_Q.ptr, storage_pool)

    Q = polynomial_Q.ptr.link

    # while Q.ABC > 0:
    #     print(Q)
    #     Q = Q.link

    print(polynomial_Q)
