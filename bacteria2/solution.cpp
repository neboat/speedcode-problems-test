#include "solution.hpp"
#include <cilk/cilk.h>

void solution_entry(const int *const *A, int **B, int N) {
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
