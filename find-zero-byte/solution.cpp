#include "solution.hpp"
#include <inttypes.h>
#include <emmintrin.h>

// Find the first zero byte in the array.
const uint8_t* solution_entry(
		uint8_t needle,
		const uint8_t* start, 
		const uint8_t* end) {
	for (; start < end; ++start) {
		if (*start == needle) {
			return start;
		}
	}
	return nullptr;
}


