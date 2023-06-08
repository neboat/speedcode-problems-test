#include <cilk/cilk.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#include <nanobench.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <emmintrin.h>

#include "./solution.hpp"


#define haszero(v) (((v) - 0x01010101UL) & ~(v) & 0x80808080UL)
#define haszero_64b(v) (((v) - 0x0101010101010101UL) & ~(v) & 0x8080808080808080UL)
#define hasvalue(x,n) \
	(haszero((x) ^ (~0UL/255 * (n))))

//const uint8_t* original_solution_tier1(
//		    uint8_t needle,
//		        const uint8_t* start, 
//			    const uint8_t* end) {
//	  for (; start < end; start += 8) {
//		      if(hasvalue(*start, needle)) {
//                         for (int i = 0; i < 8; i++) {
//                            if (start[i] == needle) return start+i;
//			 }
//		      }
//		      //if (*start == needle) {
//		      //              return start;
//		      //  	        }
//		      //  }
//		      }
//	    return nullptr;
//}

__attribute__((weak))
const uint8_t* original_solution_tier1(
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

__attribute__((weak))
const uint8_t* original_solution_tier2(uint8_t needle, const uint8_t* start, const uint8_t* end) {
	  for (; start < end; start += 4) {
		      if(haszero(*reinterpret_cast<const uint32_t*>(start))) {
                         for (int i = 0; i < 4; i++) {
                            if (start[i] == needle) return start+i;
			 }
		      }
          }
	  return nullptr;
}

__attribute__((weak))
const uint8_t* original_solution_tier3(uint8_t needle, const uint8_t* start, const uint8_t* end) {
	  for (; start < end; start += 8) {
		      if(haszero_64b(*reinterpret_cast<const uint64_t*>(start))) {
                         for (int i = 0; i < 8; i++) {
                            if (start[i] == needle) return start+i;
			 }
		      }

          }
	  return nullptr;
}

__attribute__((weak))
const uint8_t* original_solution_tier4(uint8_t needle, const uint8_t* start, const uint8_t* end) {
	__m128i needles = _mm_set1_epi8(needle);
	for (; start < end; start += 16) {
		__m128i haystack = _mm_loadu_si128(reinterpret_cast<const __m128i*>(start));
		int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(needles, haystack));
		if (mask != 0) {
			return start + __builtin_ctzl(mask);
	        }
	}
	return nullptr;
}


TEST_CASE("Correctness", "[correctness]") {
  size_t N = 10000*16;
  std::vector<uint8_t> data(N);
  ankerl::nanobench::Rng rng;
  size_t range = 4096;
  for (int i = 0; i < N; i++) {
     data[i] = rng.bounded(256);
  }
  auto reference = original_solution_tier1;
  auto solution4 = solution_entry;

  for (int i = 0; i < 1024; i++) {
    size_t offset = 16 * rng.bounded(N / 16);
    auto ret_ref =
        reference(0, (&data[0]) + offset,
                  std::min((&data[0]) + range, (&data[0]) + data.size()));
    auto ret_sol =
        solution_entry(0, (&data[0]) + offset,
                       std::min((&data[0]) + range, (&data[0]) + data.size()));
    REQUIRE(ret_ref == ret_sol);
  }
}

// Custom structure for setting up performance tiers for the
// measure_tiers() method.
struct input_t {
  const size_t N = 1 << 28;
  std::vector<uint8_t> data;
  size_t range = 0;
  int iters = 0;

  std::random_device dev;
  ankerl::nanobench::Rng rng;

  input_t(size_t _range = 4096, int _iters = 4096)
      : data(N), range(_range), iters(_iters), rng(dev()) {
    // Initialize the data with random nonzero values.
    for (size_t i = 0; i < N; ++i) {
      data[i] = 1 + rng.bounded(255);
    }
  }
  ~input_t() {}

  std::string current_size_description() const {
    return std::to_string(range);
  }

  bool grow_input() {
    range = range * 14 / 10;
    iters = iters * 14 / 10;
    return range <= N;
  }

  void run() {
    // Run multiple iterations, since each random placement of a 0 in
    // the range will affect the running time.
    for (int i = 0; i < iters; ++i) {
      size_t offset = 16 * rng.bounded((N - range) / 16);

      // Set a zero byte within the range.
      size_t zero_loc = rng.bounded(range);
      uint8_t saved = data[offset + zero_loc];
      data[offset + zero_loc] = 0;

      const uint8_t *result =
          solution_entry(0, (&data[0]) + offset, (&data[0]) + offset + range);

      data[offset + zero_loc] = saved;
    }
  }
};

const int TIER_TIMEOUT_MS = 100;
const int NUM_EPOCHS = 3;

// General method for measuring performance tiers.
//
// The template type IN_T must be a default-constructible object that
// supports the following methods:
//
// - grow_input(): Increase the input size for the next tier.
// - current_size_description: Get an std::string describing this tier.
// - run: Run solution_entry() on the current tier's input.
template <typename IN_T> void measure_tiers() {
  IN_T input;

  ankerl::nanobench::Bench b;
  b.epochs(NUM_EPOCHS);
  b.warmup(1);

  const double target =
      std::chrono::duration<double>(std::chrono::milliseconds(TIER_TIMEOUT_MS))
          .count();
  double result_tier = 0.0;
  int tier = 1;
  while (true) {
    b.run(input.current_size_description().c_str(), [&]() { input.run(); });

    const std::vector<ankerl::nanobench::Result> results = b.results();
    double result =
        results[tier - 1].median(ankerl::nanobench::Result::Measure::elapsed);

    if (result > target) {
      result_tier = tier - 1;
      // Add some fractional tier based on how close this running time
      // is to the next tier.
      double overhead = target / result;
      result_tier += (overhead * overhead);
      break;
    }

    ++tier;
    if (!input.grow_input()) {
      // printf("Maximum tier reached!\n");
      REQUIRE("Maximum tier reached!");
      result_tier = tier;
      break;
    }
  }

  // This assertion should never fail, but is included to ensure the
  // report picks it up (using the -s flag).
  REQUIRE(result_tier);

  ankerl::nanobench::render(
      "{{#result}} {{title}} {{name}} {{median(elapsed)}} {{median(pagefaults)}} "
      "{{median(branchinstructions)}} {{median(branchmisses)}}\n",
      b, std::cout);
  std::string filename{"tier1.json"};
  std::fstream s{filename, s.trunc | s.in | s.out};
  ankerl::nanobench::render(ankerl::nanobench::templates::json(), b, s);
}

TEST_CASE("Tiers", "[tier1]") {
  measure_tiers<input_t>();
}
