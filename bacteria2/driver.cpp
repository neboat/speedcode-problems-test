#include <cilk/cilk.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#include <nanobench.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "./solution.hpp"

static void moveAndSpread_reference(const int *const *A, int **B, int N) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      B[i][j] = 0;
      int x = N - j - 1;
      int y = N - i - 1;
      for (int ii = x - 1; ii <= x + 1; ii += 2) {
        for (int jj = y - 1; jj <= y + 1; jj += 2) {
          if (0 <= ii && ii < N && 0 <= jj && jj < N) {
            B[i][j] += A[ii][jj];
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
    // next_char = getchar();
    next_char = std::fgetc(fp);
  while (next_char >= '0' && next_char <= '9') {
    (*int_input) = ((*int_input) << 1) + ((*int_input) << 3) + next_char - '0';
    // next_char = getchar();
    next_char = std::fgetc(fp);
  }
}

static void readInput(std::FILE *fp, int N, int **A) {
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      fast_input(fp, &A[i][j]);
    }
  }
}

TEST_CASE("Correctness", "[correctness]") {
  // Test inputs.
  std::string test_inputs[] = {
      "_matrices/test1.in", "_matrices/test2.in", "_matrices/test3.in",
      "_matrices/test4.in", "_matrices/test5.in"};

  for (std::string input : test_inputs) {
    // Open input file
    std::FILE *fp = std::fopen(input.c_str(), "r");
    if (!fp)
      std::perror((std::string("Failed to open file ") + input).c_str());
    REQUIRE(fp);

    // Get the matrix size from the input file.
    int N;
    fscanf(fp, "%d", &N);

    // Allocate matrices.
    int **A, **B, **B_ref;
    A = new int*[N];
    B = new int*[N];
    B_ref = new int*[N];
    for (int i = 0; i < N; ++i) {
      A[i] = new int[N];
      B[i] = new int[N];
      B_ref[i] = new int[N];
    }

    // Read the input matrix
    readInput(fp, N, A);

    std::fclose(fp);

    // Run the reference and given solutions.
    moveAndSpread_reference(A, B_ref, N);
    solution_entry(A, B, N);

    // Check that the given solution produces the same result as the
    // reference.
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
	REQUIRE(B_ref[i][j] == B[i][j]);
      }
    }

    // Free the matrices.
    for (int i = 0; i < N; ++i) {
      delete[] A[i];
      delete[] B[i];
      delete[] B_ref[i];
    }
    delete[] A;
    delete[] B;
    delete[] B_ref;
  }
}

TEST_CASE("Tiers", "[tiers]") {
  
}
