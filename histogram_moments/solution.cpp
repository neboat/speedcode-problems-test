#include "solution.hpp"
#include <cilk/cilk.h>
// Note: These details below are not very important for the current formulation of the problem.
//       In the enhanced version of the problem, you would be asked to compute moments for
//       multiple subranges of data and be allowed a small amount of preprocessing when initially
//       injesting the data array. This makes possible additional optimizations, and the constraints
//       outlined below give the necessary information/clues to implement the optimization.
//
// Data derived from a grayscale image with 1-byte per pixel. There is a 1-1 mapping of floating point
// values to pixels, but you do not know the mapping.
// Each element of data represents the floating point representation for that one pixel.
// Return: a histogram of length K where the ith element of histogram contains the ith moment
// of the data.
std::vector<double> solution_entry(std::vector<double>& data, int K) {
  std::vector<double> histogram(K);
  const double thresh = 1e-5/data.size();
  for (size_t i = 0; i < data.size(); i++) {
    double x = 1.0;
    for (int j = 0; j < K; j++) {
      histogram[j] += x;//std::pow(data[i], j);
      x *= data[i];
      if (x < thresh) break;
    } 
  }
  return histogram;
}




