#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <cilk/cilk.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdint>

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


TEST_CASE("Tiers", "[correctness],[tier1]") {
  size_t N = 10000*16;
  std::vector<uint8_t> data(N);
  ankerl::nanobench::Rng rng;
  size_t range = 4096;
  for (int i = 0; i < N; i++) {
     data[i] = rng.bounded(256);
     //if (rng.bounded(range) == 0 ) {
     //  data[i] = 0;
     //}
     //if (i == N/2) data[i] = 0;
  }
  auto reference = original_solution_tier1;
  auto solution = original_solution_tier2;
  auto solution2 = original_solution_tier3;
  auto solution3 = original_solution_tier4;
  auto solution4 = solution_entry;

  for (int i = 0; i < 1024; i++) {
    size_t offset = 16*rng.bounded(N / 16);
    auto ret_ref = reference(0, (&data[0])+offset, std::min((&data[0]) + range, (&data[0]) + data.size()));
    auto ret_sol = solution(0, (&data[0])+offset, std::min((&data[0]) + range,(&data[0]) + data.size()));
    auto ret_sol2 = solution2(0, (&data[0])+offset, std::min((&data[0]) + range, (&data[0])+data.size()));
    auto ret_sol3 = solution3(0, (&data[0])+offset, std::min((&data[0]) + range, (&data[0]) + data.size()));
    REQUIRE(ret_ref == ret_sol);
    REQUIRE(ret_sol == ret_sol2);
    REQUIRE(ret_sol2 == ret_sol3);
  }



  auto bencher = ankerl::nanobench::Bench().epochs(10).minEpochIterations(2).performanceCounters(true).relative(true).minEpochTime(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(200))).run("Tier0", [&] {
                        size_t offset = 16*rng.bounded(N/16);
			auto ret = reference(0, (&data[0])+offset, std::min((&data[0]) + offset + range, (&data[0]) +data.size()));

		});
  bencher.run("Tier1", [&] {
		        size_t offset = 16*rng.bounded(N/16);
			auto ret = solution(0, (&data[0])+offset, std::min((&data[0]) + offset + range, (&data[0]) + data.size()));
			          });
  bencher.run("Tier2", [&] {
		        size_t offset = 16*rng.bounded(N/16);
			auto ret = solution2(0, (&data[0])+offset, std::min((&data[0]) + offset + range, (&data[0]) + data.size()));
			          });
  bencher.run("Tier3", [&] {
		        size_t offset = 16*rng.bounded(N/16);
			auto ret = solution3(0, (&data[0])+offset, std::min((&data[0]) + offset + range, (&data[0]) + data.size()));
			          });
  bencher.run("Submission", [&] {
		        size_t offset = 16*rng.bounded(N/16);
			auto ret = solution4(0, (&data[0])+offset, std::min((&data[0]) + offset + range, (&data[0]) + data.size()));
			          });

  auto results = bencher.results();

  std::vector<double> runtimes;
  for (int i = 0; i < results.size(); i++) {
     runtimes.push_back(results[i].average(ankerl::nanobench::Result::Measure::elapsed));
  }
  REQUIRE(runtimes[0]/runtimes[1] >= 0.95);
}

