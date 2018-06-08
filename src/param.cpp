// Copyright [2016] <Zewei Chen>
// ------By Zewei Chen------
// Email:czwking1991@gmail.com
#include <param.h>

uint Param::get_method_num() { return test_attributes.size(); }

uint Param::get_test_method(uint index) {
  return test_attributes[index].test_method;
}

uint Param::get_test_type(uint index) {
  return test_attributes[index].test_type;
}
