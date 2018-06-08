// Copyright [2016] <Zewei Chen>
// ------By Zewei Chen------
// Email:czwking1991@gmail.com
#include <random_gen.h>
#include <iostream>

// default_random_engine
//     Random_Gen::generator(std::chrono::system_clock::now().time_since_epoch()
//      .count());
std::random_device rd;
std::mt19937 Random_Gen::generator(rd());

double Random_Gen::exponential_gen(double lambda) {
  exponential_distribution<double> distribution(lambda);
  return distribution(generator);
}

int Random_Gen::uniform_integral_gen(int min, int max) {
  uniform_int_distribution<int> distribution(min, max);
  return distribution(generator);
}

ulong Random_Gen::uniform_ulong_gen(ulong min, ulong max) {
  uniform_int_distribution<ulong> distribution(min, max);
  return distribution(generator);
}

double Random_Gen::uniform_real_gen(double min, double max) {
  uniform_real_distribution<double> distribution(min, max);
  return distribution(generator);
}

bool Random_Gen::probability(double prob) {
  int i = 1, j = 0;
  if (1 < prob) prob = 1;
  while (prob != floor(prob) && j++ < 3) {
    prob *= 10;
    i *= 10;
  }
  prob = floor(prob);
  if (uniform_integral_gen(1, i) <= prob)
    return true;
  else
    return false;
}
