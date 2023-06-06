#include "solution.hpp"
#include <cilk/cilk.h>

void solution_entry(const int *A, int *B, int64_t N) {
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
