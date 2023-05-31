#include "solution.hpp"
#include <inttypes.h>
#include <emmintrin.h>
#include <immintrin.h>

// Merge two sorted arrays into output array. 
int solution_entry(const int *left_arr, int left_size, const int *right_arr, int right_size, int *output) {
  std::merge(left_arr, left_arr + left_size, right_arr, right_arr + right_size, output);
  return left_size + right_size;
}



