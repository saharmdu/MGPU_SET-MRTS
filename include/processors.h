// Copyright [2016] <Zewei Chen>
// ------By Zewei Chen------
// Email:czwking1991@gmail.com
#ifndef INCLUDE_PROCESSORS_H_
#define INCLUDE_PROCESSORS_H_

#include <types.h>
#include <set>
#include <vector>

// using namespace std;
using std::set;
using std::vector;

class Task;
class TaskSet;
class DAG_Task;
class DAG_TaskSet;
class Resource;
class ResourceSet;
class Param;

class Processor {
 private:
  uint processor_id;
  double speedfactor;
  fraction_t utilization;
  fraction_t resource_utilization;
  fraction_t density;
  bool tryed_assign;
  // TaskQueue tQueue;
  // ResourceQueue rQueue;
  set<uint> tQueue;
  set<uint> rQueue;
  TaskSet* tasks;
  ResourceSet* resources;
  double power;

 public:
  explicit Processor(uint id, double speedfactor = (1.0));
  ~Processor();
  uint get_processor_id() const;
  double get_speedfactor() const;
  void set_speedfactor(double sf);
  fraction_t get_utilization();
  fraction_t get_NCS_utilization();
  fraction_t get_density();
  fraction_t get_resource_utilization();
  bool get_tryed_assign() const;
  const set<uint>& get_taskqueue();
  bool add_task(uint t_id);
  bool remove_task(uint t_id);
  const set<uint>& get_resourcequeue();
  bool add_resource(uint r_id);
  bool remove_resource(uint r_id);
  void init();
  void update(TaskSet* tasks, ResourceSet* resources);
  double get_power();
  double get_power_heterogeneous();
};

typedef vector<Processor> Processors;

class ProcessorSet {
 private:
  Processors processors;

 public:
  ProcessorSet();
  // for identical multiprocessor platform
  explicit ProcessorSet(Param param);
  uint get_processor_num() const;
  Processors& get_processors();
  void init();
  void sort_by_task_utilization(uint dir);
  void sort_by_resource_utilization(uint dir);
  void sort_by_speedfactor(uint dir);
  void update(TaskSet* tasks, ResourceSet* resources);
  double get_total_power();
  double get_total_power_heterogeneous();
};

#endif  // INCLUDE_PROCESSORS_H_
