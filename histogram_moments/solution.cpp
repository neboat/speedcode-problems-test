#include "solution.hpp"
#include <cilk/cilk.h>

// Data derived from a grayscale image with 1-byte per pixel. There is a 1-1 mapping of floating point
// values to pixels: (uchar) i --> i/255;
// Each element of data represents the floating point representation for that one pixel.
// Return: a histogram of length K where the ith element of histogram contains the ith moment
// of the data.
std::vector<double> solution_entry(std::vector<double>& data, int K) {
  std::vector<double> histogram(K);
  for (size_t i = 0; i < data.size(); i++) {
    for (int j = 0; j < K; j++) {
      histogram[j] += std::pow(data[i], j);
    } 
  }
  return histogram;
}




