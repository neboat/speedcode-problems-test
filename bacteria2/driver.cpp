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

#include "./solution.hpp"

static void solution_reference(const int *A, int *B, int64_t N) {
  for (int64_t i = 0; i < N; i++) {
    for (int64_t j = 0; j < N; j++) {
      B[i * N + j] = 0;
      int64_t x = N - j - 1;
      int64_t y = N - i - 1;
      for (int64_t ii = x - 1; ii <= x + 1; ii += 2) {
        for (int64_t jj = y - 1; jj <= y + 1; jj += 2) {
          if (0 <= ii && ii < N && 0 <= jj && jj < N) {
            B[i * N + j] += A[ii * N + jj];
          }
        }
      }
    }
  }
}

static void fast_input(std::FILE *fp, int *int_input) {
  *int_input = 0;
  char next_char = 0;
  while (next_char < '0' || next_char > '9') // Skip non-digits
    next_char = std::fgetc(fp);
  while (next_char >= '0' && next_char <= '9') {
    (*int_input) = ((*int_input) << 1) + ((*int_input) << 3) + next_char - '0';
    next_char = std::fgetc(fp);
  }
}

static void readInput(std::FILE *fp, int64_t N, int *A) {
  for (int64_t i = 0; i < N; i++) {
    for (int64_t j = 0; j < N; j++) {
      fast_input(fp, &A[i * N + j]);
    }
  }
}

TEST_CASE("Correctness", "[correctness]") {
  // Test inputs.
  std::string test_inputs[] = {
      "_matrices/test1.in", "_matrices/test2.in", "_matrices/test3.in",
      "_matrices/test4.in", "_matrices/test5.in"};

  for (std::string input : test_inputs) {
    CAPTURE(input);

    // Open input file
    std::FILE *fp = std::fopen(input.c_str(), "r");
    if (!fp)
      std::perror((std::string("Failed to open file ") + input).c_str());
    CAPTURE(strerror(errno));
    REQUIRE(fp);

    // Get the matrix size from the input file.
    int64_t N;
    fscanf(fp, "%ld", &N);

    // Allocate matrices.
    int *A, *B, *B_ref;
    A = new int[N*N];
    B = new int[N*N];
    B_ref = new int[N*N];

    // Read the input matrix
    readInput(fp, N, A);

    std::fclose(fp);

    // Run the reference and given solutions.
    solution_reference(A, B_ref, N);
    solution_entry(A, B, N);

    // Check that the given solution produces the same result as the
    // reference.
    for (int64_t i = 0; i < N; ++i) {
      for (int64_t j = 0; j < N; ++j) {
	CAPTURE(i, j, B_ref[i * N + j], B[i * N + j]);
	REQUIRE(B_ref[i * N + j] == B[i * N + j]);
      }
    }

    // Free the matrices.
    delete[] A;
    delete[] B;
    delete[] B_ref;
  }
}

const int TIER_TIMEOUT_MS = 200;
const int NUM_EPOCHS = 3;
const std::chrono::nanoseconds MAX_BENCH_TIME =
    std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::milliseconds(TIER_TIMEOUT_MS * 2));

struct input_t {
  const int MAX_N = 1 << 13;
  int64_t N = 0;
  int *A = nullptr;
  int *B = nullptr;

  input_t(int64_t _N = 32) : N(_N) {
    std::random_device dev;
    ankerl::nanobench::Rng rng(dev());
    std::uniform_int_distribution<int> distrib(0, 999);

    A = new int[MAX_N * MAX_N];
    B = new int[MAX_N * MAX_N];

    // Randomly initialize A.
    for (int64_t i = 0; i < MAX_N; ++i) {
      for (int64_t j = 0; j < MAX_N; ++j) {
	A[i * N + j] = distrib(rng);
      }
    }
  }
  ~input_t() {
    delete[] A;
    delete[] B;
  }

  bool grow_input() {
    N *= 2;
    return N <= MAX_N;
  }

  void run() {
    solution_entry(A, B, N);
    solution_entry(B, A, N);
    solution_entry(A, B, N);
    solution_entry(B, A, N);
    solution_entry(A, B, N);
    solution_entry(B, A, N);
    solution_entry(A, B, N);
    solution_entry(B, A, N);
  }
};

TEST_CASE("Tiers", "[tier1]") {
  input_t input;

  ankerl::nanobench::Bench b;
  b.epochs(NUM_EPOCHS);
  b.warmup(1);

  int tier = 1;
  while (true) {
    b.run(std::to_string(input.N).c_str(), [&]() { input.run(); });

    const std::vector<ankerl::nanobench::Result> results = b.results();
    double result =
        results[tier - 1].median(ankerl::nanobench::Result::Measure::elapsed);

    if (result > std::chrono::duration<double>(
                     std::chrono::milliseconds(TIER_TIMEOUT_MS))
                     .count())
      break;

    ++tier;
    if (!input.grow_input()) {
      printf("Maximum tier reached!\n");
      break;
    }
  }

  ankerl::nanobench::render(
      "{{#result}} {{title}} {{median(elapsed)}} {{median(pagefaults)}} "
      "{{median(branchinstructions)}} {{median(branchmisses)}}\n",
      b, std::cout);
  std::string filename{"tier1.json"};
  std::fstream s{filename, s.trunc | s.in | s.out};
  ankerl::nanobench::render(ankerl::nanobench::templates::json(), b, s);
}
