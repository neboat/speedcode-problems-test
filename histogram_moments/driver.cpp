#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <cilk/cilk.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdint>

#include <nanobench.h>

const double ERROR_THRESH = 1e-6;


const double RUNTIME_SANITY_RATIO = 0.5;
const double RUNTIME_TEST_RATIO = 0.95;
const std::chrono::nanoseconds MAX_BENCH_EPOCHS_SANITY = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(200));
const std::chrono::nanoseconds MAX_BENCH_EPOCHS = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(200));
const int NUM_BENCH_EPOCHS=4;
const int NUM_BENCH_EPOCHS_SANITY=2;

unsigned long long todval (struct timeval *tp) {
    return tp->tv_sec * 1000 * 1000 + tp->tv_usec;
}

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "./solution.hpp"

__attribute__((weak)) std::vector<double> original_solution_tier1(std::vector<double>& data, int K) {
  std::vector<double> histogram(K); 
  for (size_t i = 0; i < data.size(); i++) {
    double x = 1.0;
    for (int j = 0; j < K; j++) {
      histogram[j] += x;//std::pow(data[i], j);
      x *= data[i];
    } 
  }
  return histogram;
}

__attribute__((weak)) std::vector<double> original_solution_tier2(std::vector<double>& data, int K) {
  std::vector<double> histogram(K);
  for (size_t i = 0; i < data.size(); i++) {
    double x = 1.0;
    for (int j = 0; j < K; j++) {
      histogram[j] += x;//std::pow(data[i], j);
      x *= data[i];
      if (x < ERROR_THRESH/data.size()) break;
    } 
  }
  return histogram;
}

__attribute__((weak)) std::vector<double> original_solution(std::vector<double>& data, int K) {
  std::vector<double> histogram(K); 
  for (size_t i = 0; i < data.size(); i++) {
    double x = 1.0;
    for (int j = 0; j < K; j++) {
      histogram[j] += x;//std::pow(data[i], j);
      x *= data[i];
      if (x < 1e-9) break;
    } 
  }
  return histogram;
}



TEST_CASE("Small, all-zeros", "[correctness]") {
  int N_data = 100;
  std::vector<double> data(N_data);
  for (int i = 0; i < N_data; i++) {
      data[i] = 0;
  }
  {
    std::vector<double> res = solution_entry(data, 1);
    std::vector<double> res_ref = original_solution(data, 1);
    REQUIRE(res.size() == res_ref.size());
    for (int i = 0; i < 1; i++) {
        INFO("Check histogram entry i=" << std::to_string(i));
        REQUIRE(res[i] == res_ref[i]);
    }
  }
  {
    std::vector<double> res = solution_entry(data, 100);
    std::vector<double> res_ref = original_solution(data, 100);
    REQUIRE(res.size() == res_ref.size());
    for (int i = 0; i < 100; i++) {
        INFO("Check histogram entry i=" << std::to_string(i));
        REQUIRE(res[i] == res_ref[i]);
    }
  }
}


TEST_CASE("Small, all-ones", "[correctness],[small]") {
  int N_data = 100; //NOTE(TFK)
  std::vector<double> data(N_data);
  for (int i = 0; i < N_data; i++) {
      data[i] = 1.0;
  }

  SECTION("K=1") {
    int K = 1;
    CAPTURE(data,K);
    std::vector<double> res = solution_entry(data, K);
    std::vector<double> res_ref = original_solution(data, K);
    REQUIRE(res.size() == res_ref.size());
    for (int i = 0; i < 1; i++) {
        //INFO("Check histogram entry i=" << std::to_string(i));
        CAPTURE(i, res[i], res_ref[i], res_ref[i] == res[i]);
        REQUIRE(res[i] == res_ref[i]);
    }
  }
  
  SECTION("K=100") {
    int K = 100;
    CAPTURE(data,K);
    std::vector<double> res = solution_entry(data, K);
    std::vector<double> res_ref = original_solution(data, K);
    REQUIRE(res.size() == res_ref.size());
    for (int i = 0; i < 100; i++) {
        //INFO("Check histogram entry i=" << std::to_string(i));
        CAPTURE(i, res[i], res_ref[i], res_ref[i] == res[i]);
        REQUIRE(res[i] == res_ref[i]);
    }
  }
}



//TEST_CASE("Correctness test", "[correctness]") {
//  int N_data = 10000000;
//  std::vector<double> data(N_data);
//  for (int i = 0; i < N_data; i++) {
//      data[i] = (i%256)*1.0 / 256;
//  }
//  std::vector<double> res = solution_entry(data, 5);
//  std::vector<double> res_ref = original_solution(data, 5);
//  REQUIRE(res.size() == res_ref.size());
//  for (int i = 0; i < 5; i++) {
//      INFO("Check histogram entry i=" << std::to_string(i));
//      REQUIRE(res[i] == res_ref[i]);
//  }
//}



//BENCHMARK_ADVANCED("Benchmark")(Catch::Benchmark::Chronometer meter) {
//  int N_data = 1000000;
//  int K = 5;
//  std::vector<double> data(N_data);
//  for (int i = 0; i < N_data; i++) {
//      data[i] = (i%256)*1.0 / 256;
//  }
//
//  std::vector<double> res_ref = original_solution(data, K);
//
//  meter.measure([&] { solution_entry(data, K) });
//
//  std::vector<double> res = solution_entry(data, K);
//  for (int i = 0; i < K; i++) {
//      CHECK(res_ref[i] == res[i]);
//  }
//};


TEST_CASE("Benchmark code", "[!benchmark],[small]") {
{
  int N_data = 1000000;
  std::vector<double> data(N_data);
  for (int i = 0; i < N_data; i++) {
      data[i] = (i%256)*1.0 / 256;
  }

  {
  	std::vector<double> res = solution_entry(data, 5);
  	std::vector<double> res_ref = original_solution(data, 5);
  	REQUIRE(res.size() == res_ref.size());
  	for (int i = 0; i < 5; i++) {
  	    REQUIRE(res[i] == res_ref[i]);
  	}

  	BENCHMARK("benchmark_small (10^6, K=5)") {
  	  return solution_entry(data, 5);
  	};
  }

  {
  	std::vector<double> res = solution_entry(data, 50);
  	std::vector<double> res_ref = original_solution(data, 50);
  	REQUIRE(res.size() == res_ref.size());
  	for (int i = 0; i < 50; i++) {
  	    REQUIRE_THAT(res[i],  Catch::Matchers::WithinRel(res_ref[i],1e-4)); 
  	}

  	BENCHMARK("benchmark_small (10^6, K=50)") {
  	  return solution_entry(data, 50);
  	};
  }



  {
  	std::vector<double> res = solution_entry(data, 100);
  	std::vector<double> res_ref = original_solution(data, 100);
  	REQUIRE(res.size() == res_ref.size());
  	for (int i = 0; i < 100; i++) {
  	    REQUIRE_THAT(res[i],  Catch::Matchers::WithinRel(res_ref[i],1e-4)); 
	}

  	BENCHMARK("benchmark_small (10^6, K=100)") {
  	  return solution_entry(data, 100);
  	};
  }

  //{
  //	std::vector<double> res = solution_entry(data, 1000);
  //	std::vector<double> res_ref = original_solution(data, 1000);
  //	REQUIRE(res.size() == res_ref.size());
  //	for (int i = 0; i < 1000; i++) {
  //	    REQUIRE_THAT(res[i],  Catch::Matchers::WithinRel(res_ref[i],1e-4)); 
  //      }

  //	BENCHMARK("benchmark_small (10^6, K=1000)") {
  //	  return solution_entry(data, 1000);
  //	};
  //}


}

}

TEST_CASE("Tier 1", "[tier1]") {

  auto reference = original_solution_tier1;
  auto solution = solution_entry;

  {  
  size_t N_data = 100000000uL/100;
  int K = 100;
  std::vector<double> data(N_data);
  for (size_t i = 0; i < N_data; i++) {
      data[i] = (i%256)*1.0 / 256;
  }
  std::vector<double> res = solution(data, K);

  std::vector<double> res_ref = reference(data, K);
  REQUIRE(res.size() == res_ref.size());
  for (int i = 0; i < K; i++) {
      //REQUIRE(res[i] == res_ref[i]);
      REQUIRE_THAT(res[i],  Catch::Matchers::WithinRel(res_ref[i],ERROR_THRESH));
  }

  //auto bencher = ankerl::nanobench::Bench().epochs(NUM_BENCH_EPOCHS).minEpochIterations(2).maxEpochTime(MAX_BENCH_EPOCHS).performanceCounters(true).relative(true).warmup(0).run("Target performance", [&] {
  auto bencher = ankerl::nanobench::Bench().epochs(NUM_BENCH_EPOCHS_SANITY).epochs(1).epochIterations(2).warmup(0).run("Target performance", [&] {
		                                auto ret = solution(data, K);
						                                  });
  bencher.epochs(1).run("MyBenchmarkTest", [&] {
		              auto ret = solution(data, K);
			      printf("new run\n");
			          });

  auto results = bencher.results();

  std::vector<double> runtimes;
  for (int i = 0; i < results.size(); i++) {
     printf("Result %d has time %f\n", i, results[i].average(ankerl::nanobench::Result::Measure::elapsed));
     runtimes.push_back(results[i].average(ankerl::nanobench::Result::Measure::elapsed));
  }
  REQUIRE(runtimes[0]/runtimes[1] >= RUNTIME_SANITY_RATIO);
  }

  {  
  size_t N_data = 100000000uL/100;
  int K = 100;
  std::vector<double> data(N_data);
  for (size_t i = 0; i < N_data; i++) {
      data[i] = (i%256)*1.0 / 256;
  }
  std::vector<double> res = solution(data, K);

  std::vector<double> res_ref = reference(data, K);
  REQUIRE(res.size() == res_ref.size());
  for (int i = 0; i < K; i++) {
      //REQUIRE(res[i] == res_ref[i]);
      REQUIRE_THAT(res[i],  Catch::Matchers::WithinRel(res_ref[i],ERROR_THRESH));
  }

  auto bencher = ankerl::nanobench::Bench().epochs(NUM_BENCH_EPOCHS).minEpochIterations(2).maxEpochTime(MAX_BENCH_EPOCHS).performanceCounters(true).relative(true).minEpochTime(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(200))).run("Target performance (Tier 1)", [&] {
		                                auto ret = reference(data, K);
						                                  });
  bencher.run("MyBenchmarkTest", [&] {
		              auto ret = solution(data, K);
			          });

  auto results = bencher.results();

  std::vector<double> runtimes;
  for (int i = 0; i < results.size(); i++) {
     printf("Result %d has time %f\n", i, results[i].average(ankerl::nanobench::Result::Measure::elapsed));
     runtimes.push_back(results[i].average(ankerl::nanobench::Result::Measure::elapsed));
  }
  REQUIRE(runtimes[0]/runtimes[1] >= RUNTIME_TEST_RATIO);
  ankerl::nanobench::render("{{#result}} {{sum(iterations)}} {{title}} {{average(elapsed)}} {{average(pagefaults)}} {{average(branchinstructions)}} {{average(branchmisses)}}\n", bencher, std::cout);
  std::string filename{"tier1.json"};
  std::fstream s{filename, s.trunc | s.in | s.out};
  ankerl::nanobench::render(ankerl::nanobench::templates::json(), bencher, s);//("{{#result}} {{title}} {{average(elapsed)}} {{average(pagefaults)}} {{average(branchinstructions)}} {{average(branchmisses)}}\n", bencher, std::cout);
  }


  //BENCHMARK("benchmark_large (10^8)") {
  //  return solution_entry(data, 5);
  //};
}


TEST_CASE("Tier 2", "[tier2]") {

  auto reference = original_solution_tier2;
  auto solution = solution_entry;
  {  
  size_t N_data = 100000000uL/100;
  int K = 100;
  std::vector<double> data(N_data);
  for (size_t i = 0; i < N_data; i++) {
      data[i] = (i%256)*1.0 / 256;
  }
  std::vector<double> res = solution(data, K);

  std::vector<double> res_ref = reference(data, K);
  REQUIRE(res.size() == res_ref.size());
  for (int i = 0; i < K; i++) {
      //REQUIRE(res[i] == res_ref[i]);
      REQUIRE_THAT(res[i],  Catch::Matchers::WithinAbs(res_ref[i],ERROR_THRESH));
  }

  auto bencher = ankerl::nanobench::Bench().epochs(NUM_BENCH_EPOCHS_SANITY).minEpochIterations(2).maxEpochTime(MAX_BENCH_EPOCHS).performanceCounters(true).relative(true).minEpochTime(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(200))).run("Target performance", [&] {
		                                auto ret = solution(data, K);
						                                  });
  bencher.run("MyBenchmarkTest", [&] {
		              auto ret = solution(data, K);
			          });

  auto results = bencher.results();

  std::vector<double> runtimes;
  for (int i = 0; i < results.size(); i++) {
     printf("Result %d has time %f\n", i, results[i].average(ankerl::nanobench::Result::Measure::elapsed));
     runtimes.push_back(results[i].average(ankerl::nanobench::Result::Measure::elapsed));
  }
  REQUIRE(runtimes[0]/runtimes[1] >= RUNTIME_SANITY_RATIO);
  }

  { 
  size_t N_data = 100000000uL/100;
  int K = 100;
  std::vector<double> data(N_data);
  for (size_t i = 0; i < N_data; i++) {
      data[i] = (i%256)*1.0 / 256;
  }
  std::vector<double> res = solution(data, K);

  std::vector<double> res_ref = reference(data, K);
  REQUIRE(res.size() == res_ref.size());
  for (int i = 0; i < K; i++) {
      //REQUIRE(res[i] == res_ref[i]);
      REQUIRE_THAT(res[i],  Catch::Matchers::WithinAbs(res_ref[i],ERROR_THRESH));
  }

  auto bencher = ankerl::nanobench::Bench().epochs(NUM_BENCH_EPOCHS).minEpochIterations(2).maxEpochTime(MAX_BENCH_EPOCHS).performanceCounters(true).relative(true).minEpochTime(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(200))).run("Target performance (Tier 2)", [&] {
		                                auto ret = reference(data, K);
						                                  });
  bencher.run("MyBenchmarkTest", [&] {
		              auto ret = solution(data, K);
			          });

  auto results = bencher.results();

  std::vector<double> runtimes;
  for (int i = 0; i < results.size(); i++) {
     printf("Result %d has time %f\n", i, results[i].average(ankerl::nanobench::Result::Measure::elapsed));
     runtimes.push_back(results[i].average(ankerl::nanobench::Result::Measure::elapsed));
  }
  REQUIRE(runtimes[0]/runtimes[1] >= RUNTIME_TEST_RATIO);
  ankerl::nanobench::render("{{#result}} {{title}} {{average(elapsed)}} {{average(pagefaults)}} {{average(branchinstructions)}} {{average(branchmisses)}}\n", bencher, std::cout);
  std::string filename{"tier2.json"};
  std::fstream s{filename, s.trunc | s.in | s.out};
  ankerl::nanobench::render(ankerl::nanobench::templates::json(), bencher, s);//("{{#result}} {{title}} {{average(elapsed)}} {{average(pagefaults)}} {{average(branchinstructions)}} {{average(branchmisses)}}\n", bencher, std::cout);
  }


  //BENCHMARK("benchmark_large (10^8)") {
  //  return solution_entry(data, 5);
  //};
}

