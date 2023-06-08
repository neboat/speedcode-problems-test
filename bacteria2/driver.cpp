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

static int fast_input_buffer(const char *__restrict__ buffer,
                             int *__restrict__ int_input) {
  const char *start = buffer;
  *int_input = 0;
  char next_char = 0;
  while (next_char < '0' || next_char > '9') // Skip non-digits
    next_char = *buffer++;
  while (next_char >= '0' && next_char <= '9') {
    (*int_input) = ((*int_input) << 1) + ((*int_input) << 3) + next_char - '0';
    next_char = *buffer++;
  }
  return buffer - start;
}

static void readInput_buffer(const char *buffer, int64_t N, int *A) {
  int read = 0;
  for (int64_t i = 0; i < N; i++) {
    for (int64_t j = 0; j < N; j++) {
      read += fast_input_buffer(buffer + read, &A[i * N + j]);
    }
  }
}

const std::string test1 = R"(
2 
805 378 
767 597 
)";

const std::string test2 = R"(
4 
805 378 767 597 
258 112 68 500 
771 771 590 68 
356 197 423 645 
)";

const std::string test3 = R"(
8 
805 378 767 597 258 112 68 500 
771 771 590 68 356 197 423 645 
234 238 425 123 701 407 580 850 
912 537 795 890 587 846 837 683 
3 194 292 139 710 334 293 667 
346 664 228 479 989 561 777 862 
338 755 849 288 795 958 846 678 
68 424 965 883 119 865 214 610 
)";

const std::string test4 = R"(
16 
805 378 767 597 258 112 68 500 771 771 590 68 356 197 423 645 
234 238 425 123 701 407 580 850 912 537 795 890 587 846 837 683 
3 194 292 139 710 334 293 667 346 664 228 479 989 561 777 862 
338 755 849 288 795 958 846 678 68 424 965 883 119 865 214 610 
680 253 241 780 793 669 855 372 529 832 939 813 724 225 479 495 
356 496 242 770 478 832 515 612 178 731 197 118 335 737 497 738 
289 946 853 237 923 160 625 39 580 122 869 66 750 535 442 825 
598 176 486 385 981 721 32 137 190 981 536 920 656 643 513 82 
871 792 698 213 469 357 636 875 553 964 606 85 758 977 427 211 
685 315 545 156 918 75 295 228 496 740 519 588 159 565 50 954 
858 950 950 661 793 968 559 92 475 623 996 343 108 951 180 791 
751 978 29 241 657 287 226 246 675 434 214 126 349 276 670 336 
436 810 262 434 375 453 16 507 798 873 619 249 649 211 283 607 
430 621 269 742 776 520 659 197 902 798 262 264 753 8 301 859 
760 95 927 923 430 652 906 539 74 787 35 334 349 240 508 375 
298 415 241 952 458 789 388 400 904 382 833 897 449 985 819 747 
)";

const std::string test5 = R"(
32 
805 378 767 597 258 112 68 500 771 771 590 68 356 197 423 645 234 238 425 123 701 407 580 850 912 537 795 890 587 846 837 683 
3 194 292 139 710 334 293 667 346 664 228 479 989 561 777 862 338 755 849 288 795 958 846 678 68 424 965 883 119 865 214 610 
680 253 241 780 793 669 855 372 529 832 939 813 724 225 479 495 356 496 242 770 478 832 515 612 178 731 197 118 335 737 497 738 
289 946 853 237 923 160 625 39 580 122 869 66 750 535 442 825 598 176 486 385 981 721 32 137 190 981 536 920 656 643 513 82 
871 792 698 213 469 357 636 875 553 964 606 85 758 977 427 211 685 315 545 156 918 75 295 228 496 740 519 588 159 565 50 954 
858 950 950 661 793 968 559 92 475 623 996 343 108 951 180 791 751 978 29 241 657 287 226 246 675 434 214 126 349 276 670 336 
436 810 262 434 375 453 16 507 798 873 619 249 649 211 283 607 430 621 269 742 776 520 659 197 902 798 262 264 753 8 301 859 
760 95 927 923 430 652 906 539 74 787 35 334 349 240 508 375 298 415 241 952 458 789 388 400 904 382 833 897 449 985 819 747 
640 607 482 14 250 549 62 908 249 484 676 198 987 10 327 382 418 455 287 363 556 460 355 576 724 989 224 50 355 495 683 498 
187 618 692 209 973 405 961 670 701 245 691 445 207 185 317 639 288 453 311 351 210 38 33 691 37 709 128 119 641 466 163 45 
766 297 225 225 733 513 118 263 550 709 862 467 834 619 55 921 272 223 830 123 967 960 589 371 601 104 88 713 899 586 745 864 
971 619 358 189 589 715 399 40 929 372 804 909 642 348 342 257 669 450 711 974 497 489 374 809 530 201 76 155 507 936 667 576 
725 902 871 949 864 283 335 897 944 926 161 391 670 737 831 504 548 313 676 928 42 79 519 359 985 695 321 516 566 237 609 134 
105 570 863 326 996 737 667 290 33 645 211 504 965 479 860 618 6 180 966 784 710 335 0 967 144 129 608 157 169 967 893 905 
691 689 368 837 986 148 943 460 956 447 141 87 710 301 734 25 332 407 248 662 497 931 167 566 173 520 294 188 453 785 378 972 
468 313 133 7 864 365 74 665 634 547 833 30 585 991 360 187 8 191 46 428 738 554 880 577 463 64 659 363 677 342 434 915 
466 757 363 817 959 483 105 526 567 725 32 681 849 247 906 937 679 726 762 810 760 26 643 291 950 395 627 918 920 896 842 558 
194 217 966 68 965 633 656 700 481 529 906 503 356 442 393 296 454 479 320 321 388 671 904 735 303 12 208 135 648 741 336 995 
335 838 938 678 136 670 980 649 885 983 776 274 735 615 244 363 1 429 692 165 424 435 102 538 913 826 309 340 35 371 489 286 
789 416 455 922 933 518 513 216 190 366 280 462 202 60 768 589 208 230 573 373 949 762 861 81 777 894 486 410 564 443 106 590 
499 824 911 889 195 694 717 54 916 76 783 257 465 945 864 406 785 273 864 804 308 945 299 471 598 727 552 555 904 473 690 513 
705 334 777 111 340 718 9 985 338 610 583 695 522 833 556 640 169 95 486 447 805 587 594 802 749 553 903 876 223 472 98 737 
48 512 143 457 123 578 401 758 550 544 285 226 653 181 802 123 839 721 236 923 959 141 952 620 436 773 0 722 326 766 569 432 
926 442 628 135 26 156 137 43 765 553 770 269 584 323 227 797 317 848 327 769 31 306 491 738 285 185 966 35 247 788 384 977 
759 649 945 548 766 402 402 119 638 741 491 66 704 893 144 180 488 578 987 727 311 34 775 574 699 769 453 238 314 831 749 677 
158 197 674 606 243 167 741 409 364 784 256 858 294 886 258 204 35 769 65 128 206 410 80 890 190 442 90 118 229 448 266 560 
182 879 617 976 232 374 219 491 740 641 729 908 556 504 895 523 438 845 288 791 115 708 395 298 131 951 760 237 227 340 118 667 
761 721 804 891 468 910 453 296 824 300 633 755 729 502 716 820 175 461 951 495 683 846 932 29 248 194 32 367 326 261 479 12 
829 232 573 848 552 436 892 914 708 298 542 909 582 4 766 603 579 72 500 433 67 478 225 694 448 37 732 20 326 217 633 476 
912 485 483 959 292 392 597 240 364 851 234 874 643 754 469 95 127 401 204 305 244 569 840 665 15 335 893 36 18 564 649 465 
914 422 865 795 78 401 427 988 391 221 943 12 95 692 243 441 279 671 251 466 977 998 222 368 511 366 60 957 342 575 642 476 
955 489 241 82 205 292 482 54 943 621 753 890 101 129 679 908 352 330 312 952 160 605 873 620 601 346 454 190 819 26 699 284 
)";

TEST_CASE("Correctness", "[correctness]") {
  // Test inputs.
  // std::string test_inputs[] = {
  //     "_matrices/test1.in", "_matrices/test2.in", "_matrices/test3.in",
  //     "_matrices/test4.in", "_matrices/test5.in"};
  std::string test_inputs[] = {test1, test2, test3, test4, test5};

  for (std::string input : test_inputs) {
    CAPTURE(input);

    // // Open input file
    // std::FILE *fp = std::fopen(input.c_str(), "r");
    // CAPTURE(strerror(errno));
    // if (!fp)
    //   std::perror((std::string("Failed to open file ") + input).c_str());
    // REQUIRE(fp);

    // Get the matrix size from the input file.
    int64_t N;
    // fscanf(fp, "%ld", &N);
    int pos;
    sscanf(input.c_str(), "%ld%n", &N, &pos);

    // Allocate matrices.
    int *A, *B, *B_ref;
    A = new int[N * N];
    B = new int[N * N];
    B_ref = new int[N * N];

    // Read the input matrix
    // readInput(fp, N, A);
    readInput_buffer(input.c_str() + pos, N, A);

    // std::fclose(fp);

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

  std::string current_size_description() const {
    return std::to_string(N);
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

const int TIER_TIMEOUT_MS = 200;
const int NUM_EPOCHS = 3;

// General method for measuring performance tiers.
//
// The template type IN_T must be a constructible object that supports
// the following methods:
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
