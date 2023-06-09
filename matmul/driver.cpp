#include <cilk/cilk.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#include <nanobench.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "./solution.hpp"

__attribute__((weak)) void solution_reference(float *__restrict__ C,
                                              const float *__restrict__ A,
                                              const float *__restrict__ B,
                                              int64_t m, int64_t n, int64_t l) {
  for (int64_t i = 0; i < m; ++i) {
    for (int64_t j = 0; j < n; ++j) {
      C[i * n + j] = 0;
      for (int64_t k = 0; k < l; ++k) {
	C[i * n + j] += A[i * l + k] * B[k * n + j];
      }
    }
  }
}

// Custom structure for setting up matrix-multiplication problems.
struct input_t {
  const int MAX_DIM = 1 << 13;
  int64_t m = 0, n = 0, l = 0;
  float *C = nullptr, *A = nullptr, *B = nullptr;

  std::random_device dev;
  ankerl::nanobench::Rng rng;

  input_t(int64_t _m = 512, int64_t _n = 512, int64_t _l = 512)
      : m(_m), n(_n), l(_l), rng(dev()) {
    A = new float[MAX_DIM * MAX_DIM];
    B = new float[MAX_DIM * MAX_DIM];
    C = new float[MAX_DIM * MAX_DIM];
  }
  ~input_t() {
    delete[] A;
    delete[] B;
    delete[] C;
  }

  void random_fill(float *X, int64_t m, int64_t n) {
    for (int64_t i = 0; i < m; ++i)
      for (int64_t j = 0; j < n; ++j)
        X[i * n + j] =
            (float)(rng()) / (float)(std::numeric_limits<int64_t>::max());
  }

  std::string current_size_description() const {
    return std::string(std::to_string(m) + "," +
		       std::to_string(n) + "," +
		       std::to_string(l));
  }

  bool grow_input() {
    const int64_t min_dim = std::min(std::min(m, n), l);
    if (m == min_dim)
      m = m * 16 / 10;
    else if (n == min_dim)
      n = n * 16 / 10;
    else
      l = l * 16 / 10;

    if (m > MAX_DIM || n > MAX_DIM || l > MAX_DIM)
      return false;

    random_fill(A, m, l);
    random_fill(B, l, n);

    return true;
  }

  void run() {
    solution_entry(C, A, B, m, n, l);
  }
};

static void compare_matrices(const float *C_ref, const float *C, int64_t m,
                             int64_t n) {
  for (int64_t i = 0; i < m; ++i)
    for (int64_t j = 0; j < n; ++j)
      REQUIRE(C_ref[i * n + j] == C[i * n + j]);
}

TEST_CASE("Correctness", "[correctness]") {
  input_t input(16, 16, 16);
  float *C_ref = new float[input.MAX_DIM * input.MAX_DIM];

  for (int test = 0; test < 6; ++test) {
    input.run();
    solution_reference(C_ref, input.A, input.B, input.m, input.n, input.l);
    compare_matrices(C_ref, input.C, input.m, input.n);

    input.grow_input();
    input.run();
    solution_reference(C_ref, input.A, input.B, input.m, input.n, input.l);
    compare_matrices(C_ref, input.C, input.m, input.n);

    input.grow_input();
    input.run();
    solution_reference(C_ref, input.A, input.B, input.m, input.n, input.l);
    compare_matrices(C_ref, input.C, input.m, input.n);

    input.grow_input();
  }

  delete[] C_ref;
}

const int TIER_TIMEOUT_MS = 200;
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
