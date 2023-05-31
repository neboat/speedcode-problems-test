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
#include "./ref_merge.cpp"

#define haszero(v) (((v) - 0x01010101UL) & ~(v) & 0x80808080UL)
#define haszero_64b(v) (((v) - 0x0101010101010101UL) & ~(v) & 0x8080808080808080UL)
#define hasvalue(x,n) \
	(haszero((x) ^ (~0UL/255 * (n))))


int Merge_Std(const int *aArr, int aCnt, const int *bArr, int bCnt, int *dst);
int Merge_ScalarTrivial(const int *aArr, int aCnt, const int *bArr, int bCnt, int *dst);
int Merge_ScalarOptimized_2(const int *aArr, int aCnt, const int *bArr, int bCnt, int *dst);
int Merge_ScalarOptimized_3(const int *aArr, int aCnt, const int *bArr, int bCnt, int *dst);
int Merge_ScalarBranchless_3(const int *aArr, int aCnt, const int *bArr, int bCnt, int *dst);
int Merge_Simd_KeysOnly(const int *aArr, int aCnt, const int *bArr, int bCnt, int *dst);

auto original_solution_tier1 = Merge_Std;
auto original_solution_tier2 = Merge_ScalarTrivial;
auto original_solution_tier3 = Merge_ScalarBranchless_3;
auto original_solution_tier4 = Merge_Simd_KeysOnly;

TEST_CASE("Tiers", "[correctness],[tier1]") {
  size_t N = 100000*16;
  std::vector<int32_t> data_left(N);
  std::vector<int32_t> data_right(N);
  std::vector<int32_t> output(2*N);
  ankerl::nanobench::Rng rng;
  for (int i = 0; i < N; i++) {
     data_left[i] = rng.bounded(1<<30);
     data_right[i] = rng.bounded(1<<30);
  }
  std::sort(data_left.begin(), data_left.end());
  std::sort(data_right.begin(), data_right.end());
  auto reference = original_solution_tier1;
  auto solution = original_solution_tier2;
  auto solution2 = original_solution_tier3;
  auto solution3 = original_solution_tier4;
  auto submission = solution_entry;
  //auto solution4 = solution_entry;

  //for (int i = 0; i < 1024; i++) {
  //  auto ret_ref = reference((&data[0]), data.size()/2, (&data[0])+data.size()/2, data.size()/2, (&output[0]));
  //  auto ret_sol = solution((&data[0]), data.size()/2, (&data[0])+data.size()/2, data.size()/2, (&output[0]));
  //  auto ret_sol2 = solution2((&data[0]), data.size()/2, (&data[0])+data.size()/2, data.size()/2, (&output[0]));
  //  auto ret_sol3 = solution3((&data[0]), data.size()/2, (&data[0])+data.size()/2, data.size()/2, (&output[0]));
  //  for (int i = 0; i < N; i++) {
  //      REQUIRE(ret_ref[i] == ret_sol[i]);
  //      REQUIRE(ret_sol[i] == ret_sol2[i]);
  //      REQUIRE(ret_sol2[i] == ret_sol3[i]);
  //  }
  //}

//  for (int i = 0; i < N; i++) {
//     data[i] = rng.bounded(1<<30);
//  }


  auto bencher = ankerl::nanobench::Bench().epochs(10).minEpochIterations(2).performanceCounters(true).relative(true).minEpochTime(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(200))).run("Tier0", [&] {
    			auto ret_ref = reference((&data_left[0]), data_left.size(), (&data_right[0]), data_right.size(), (&output[0]));

		});
//  for (int i = 0; i < N; i++) {
//     data[i] = rng.bounded(1<<30);
//  }
  bencher.run("Tier1", [&] {
    			auto ret_ref = solution((&data_left[0]), data_left.size(), (&data_right[0]), data_right.size(), (&output[0]));
			          });
//  for (int i = 0; i < N; i++) {
//     data[i] = rng.bounded(1<<30);
//  }
  bencher.run("Tier2", [&] {
    			auto ret_ref = solution2((&data_left[0]), data_left.size(), (&data_right[0]), data_right.size(), (&output[0]));
			          });
//  for (int i = 0; i < N; i++) {
//     data[i] = rng.bounded(1<<30);
//  }
  bencher.run("Tier3", [&] {
    			auto ret_ref = solution3((&data_left[0]), data_left.size(), (&data_right[0]), data_right.size(), (&output[0]));
			          });
  bencher.run("Submission", [&] {
    			auto ret_ref = submission((&data_left[0]), data_left.size(), (&data_right[0]), data_right.size(), (&output[0]));
			          });

  auto results = bencher.results();

  std::vector<double> runtimes;
  for (int i = 0; i < results.size(); i++) {
     runtimes.push_back(results[i].average(ankerl::nanobench::Result::Measure::elapsed));
  }
  REQUIRE(runtimes[0]/runtimes[1] >= 0.95);
}

