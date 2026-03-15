#include <stdint.h>
#include <stdbool.h>


// Algorithm

// P1
// init primes_array[0] = 2;
// init primes_found = 1;
// init candidate = 3;
// init primes_counter = 0;

// P2
// candidate is_prime:
	// primes_array[primes_found] = candidate;
	// primes_found += 1;

// P3
// If primes_found == primes_to_find - 1: return

// P4
// candidate += 2;

// P5
// primes_counter = 1. (will run through the possible prime divisors of candidate);

// P6
// primes_array[primes_counter] divides candidate?
// divide candidate by primes_array[primes_counter] and let Q = quotient, R = remainder;
	// if R == 0: N is not prime => go to P4;

// P7
// primes_array[primes_counter] large?
// If Q <= primes_array[primes_counter], go to P2. (in such case candidate must be prime)

// P8
// primes_counter += 1 and go to P6;


bool is_prime_correct(uint32_t candidate, uint32_t* primes_array) {
	uint32_t Q;
	uint32_t R;

	// P5
	uint32_t primes_counter = 0;  // start from 3 as a divisorS

	// P7
	do {
		primes_counter += 1;

		// P6
		Q = candidate / primes_array[primes_counter];
		R = candidate % primes_array[primes_counter];

		if (R == 0) {
			// to P4
			return false;
		}
	} while (Q > primes_array[primes_counter]);

	return true;
}

// 3k cycles slower than original is_prime
//bool is_prime(uint32_t candidate, uint32_t* primes_array) {
//	uint32_t Q;
//	uint32_t temp_product;
//	uint32_t divisor;
//
//	// P5
//	uint32_t primes_counter = 0;  // start from 3 as a divisor
//
//	// P7
//	do {
//		primes_counter += 1;
//		divisor = primes_array[primes_counter];
//
//		// P6
//		Q = candidate / divisor;
//
//		// THE MUL STEAL: Replace modulo (%) with Multiply (*)
//		// This forces GCC to use a 1-cycle MUL instead of a 2-cycle MLS
//		temp_product = Q * divisor;
//
//		// If Q * divisor perfectly reconstructs the candidate, remainder is 0.
//		if (temp_product == candidate) {
//			// to P4
//			return false;
//		}
//	} while (Q > divisor);
//
//	return true;
//}

// correct cold run and incorrect warm one
uint32_t* c_knuth_primes_buggy(uint32_t* primes_array, uint32_t primes_to_find) {
	// P1
	primes_array[0] = 2;
	uint32_t primes_found = 1;
	uint32_t candidate = 3;

	// P2
//	primes_array[primes_found] = candidate;
//	primes_found++;

	while (1) {
		if (is_prime_buggy(candidate, primes_array)) {
			// go to P2. (in such case candidate must be prime)
			primes_array[primes_found] = candidate;
			primes_found++;
		}

		// P3
		if (primes_found == primes_to_find) {
			return;
		}

		// P4
		candidate += 2;
	}
}
