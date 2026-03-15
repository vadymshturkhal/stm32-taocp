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


bool is_prime_mod_3_hack(uint32_t candidate, uint32_t* primes_array) {
	uint32_t Q;
	uint32_t R;

	// P5
	uint32_t primes_counter = 1;  // start from 3 as a divisorS

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

void c_knuth_primes_mod_3_hack(uint32_t* primes_array, uint32_t primes_to_find) {
	// P1
	// @ R8 0xAAAAAAAB (modular inverse of 3)
	// @ R9 0x55555555 (max valid limit for mod 3)

	const uint32_t modular_inverse_of_3 = 0xAAAAAAAB;
	const uint32_t max_valid_limit_for_mod_3 = 0x55555555;

	primes_array[0] = 2;
	primes_array[1] = 3;
	uint32_t primes_found = 2;
	uint32_t candidate = 5;

	// P2
	primes_array[primes_found] = candidate;
	primes_found++;

	while (1) {
		// P3
		if (primes_found == primes_to_find) {
			return;
		}

		// P4 with mod 3 hack
		do {
			candidate += 2;
		} while ((candidate * modular_inverse_of_3) <= max_valid_limit_for_mod_3);

		if (is_prime_mod_3_hack(candidate, primes_array)) {
			// go to P2. (in such case candidate must be prime)
			primes_array[primes_found] = candidate;
			primes_found++;
		}
	}
}
