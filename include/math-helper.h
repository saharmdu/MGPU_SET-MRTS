// Copyright [2016] <Zewei Chen>
// ------By Zewei Chen------
// Email:czwking1991@gmail.com
#ifndef INCLUDE_MATH_HELPER_H_
#define INCLUDE_MATH_HELPER_H_

#include <types.h>

static inline ulong ceiling(ulong numer, ulong denom) {
  if (numer % denom == 0)
    return (numer / denom);
  else
    return (numer / denom) + 1;
}

static inline ulong ceiling(ulong numer, double denom) {
  if (numer / denom < (static_cast<int64_t>(numer / denom) + _EPS))
    return (ulong)(numer / denom);
  else
    return (ulong)(numer / denom) + 1;
}
/*
bool double_equal(double a, double b)
{
        return fabs(a - b) < EPS;
}
*/
#endif  // INCLUDE_MATH_HELPER_H_
