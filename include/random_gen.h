// Copyright [2016] <Zewei Chen>
// ------By Zewei Chen------
// Email:czwking1991@gmail.com
#ifndef INCLUDE_RANDOM_GEN_H_
#define INCLUDE_RANDOM_GEN_H_

#include <types.h>
#include <random>

using std::uniform_int_distribution;
using std::uniform_real_distribution;
using std::exponential_distribution;

class Random_Gen {
 private:
  // static default_random_engine generator;
  static std::mt19937 generator;

 public:
  // Random_Gen();
  static double exponential_gen(double lambda);
  static int uniform_integral_gen(int min, int max);
  static ulong uniform_ulong_gen(ulong min, ulong max);
  static double uniform_real_gen(double min, double max);
  static bool probability(double prob);
};

#endif  // INCLUDE_RANDOM_GEN_H_
