#include "solution.hpp"
#include <cilk/cilk.h>

void solution_entry(float *__restrict__ C, const float *__restrict__ A,
                    const float *__restrict__ B, int64_t m, int64_t n,
                    int64_t l) {
  for (int64_t i = 0; i < m; ++i) {
    for (int64_t j = 0; j < n; ++j) {
      C[i * n + j] = 0;
      for (int64_t k = 0; k < l; ++k) {
	C[i * n + j] += A[i * l + k] * B[k * n + j];
      }
    }
  }
}
