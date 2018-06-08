// Copyright [2016] <Zewei Chen>
// ------By Zewei Chen------
// Email:czwking1991@gmail.com
#ifndef INCLUDE_PARAM_H_
#define INCLUDE_PARAM_H_

#include <types.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

class Param {
 public:
  uint id;
  // network
  const char* server_ip;
  uint port;
  // basic parameters
  uint lambda;
  uint p_num;
  double step;
  Range p_range;
  Range u_range;
  Range d_range;
  Test_Attribute_Set test_attributes;
  uint exp_times;
  // resource parameters
  uint resource_num;
  uint mcsn;
  uint rrn;
  double rrp;
  double tlf;
  Range rrr;
  // heterogenegous
  Ratio ratio;
  // graph task model parameters
  Range job_num_range;
  Range arc_num_range;
  bool is_cyclic;
  uint max_indegree;
  uint max_outdegree;
  double para_prob;
  double cond_prob;
  double arc_density;
  uint max_para_job;
  uint max_cond_branch;

 public:
  void set_lambda(int lambda);
  uint get_lambda() const;
  void set_processor_num(int num);
  uint get_processor_num() const;
  void set_step(double step);
  double get_step() const;
  void set_period_range(Range p_range);
  Range get_period_range() const;
  void set_utilization_range(Range u_range);
  Range get_utilization_range() const;
  void set_deadline_range(Range d_range);
  Range get_deadline_range() const;
  void set_test_attribute_set(Test_Attribute_Set test_attributes);
  Test_Attribute_Set get_test_attribute_set() const;
  void set_experiment_times(uint times);
  uint get_experiment_times() const;

  void set_resource_num(int resource_num);
  uint get_resource_num() const;
  void set_request_num(int rrn);
  uint get_request_num() const;
  void set_request_probability(double rrp);
  double get_request_probability() const;
  void set_total_length_factor(double tlf);
  double get_total_length_factor() const;
  void set_request_range(Range rrr);
  Range get_request_range() const;

  void set_job_num_range(Range job_num_range);
  Range get_job_num_range() const;
  void set_arc_num_range(Range arc_num_range);
  Range get_arc_num_range() const;
  void set_is_cyclic(bool is_cyclic);
  bool get_is_cyclic() const;
  void set_max_indegree(int max_indegree);
  uint get_max_indegree() const;
  void set_max_outdegree(int max_outdegree);
  uint get_ax_outdegree() const;

  uint get_method_num();
  uint get_test_method(uint index);
  uint get_test_type(uint index);
};

#endif  // INCLUDE_PARAM_H_
