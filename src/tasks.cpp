// Copyright [2017] <Zewei Chen>
// ------By Zewei Chen------
// Email:czwking1991@gmail.com
#include <iteration-helper.h>
#include <math-helper.h>
#include <math.h>
#include <param.h>
#include <processors.h>
#include <random_gen.h>
#include <resources.h>
#include <sort.h>
#include <tasks.h>
#include <xml.h>
#include <stdio.h>
#include <string>
#include <fstream>

using std::max;
using std::min;
using std::ifstream;
using std::string;
using std::to_string;
using std::exception;

/** Class Request */

Request::Request(uint resource_id, uint num_requests, ulong max_length,
                 ulong total_length, uint locality) {
  this->resource_id = resource_id;
  this->num_requests = num_requests;
  this->max_length = max_length;
  this->total_length = total_length;
  this->locality = locality;
}

uint Request::get_resource_id() const { return resource_id; }
uint Request::get_num_requests() const { return num_requests; }
ulong Request::get_max_length() const { return max_length; }
ulong Request::get_total_length() const { return total_length; }
ulong Request::get_locality() const { return locality; }
void Request::set_locality(ulong partition) { locality = partition; }

/** Class Task */

Task::Task(uint id, ulong wcet, ulong period, ulong deadline, uint priority) {
  this->id = id;
  this->index = id;
  this->wcet = wcet;
  if (0 == deadline)
    this->deadline = period;
  else
    this->deadline = deadline;
  this->period = period;
  this->priority = priority;
  utilization = this->wcet;
  utilization /= this->period;
  density = this->wcet;
  if (this->deadline <= this->period)
    density /= this->deadline;
  else
    density /= this->period;
  partition = MAX_INT;
  spin = 0;
  self_suspension = 0;
  local_blocking = 0;
  total_blocking = 0;
  jitter = 0;
  response_time = deadline;
  cluster = MAX_INT;
  independent = true;
  wcet_non_critical_sections = this->wcet;
  wcet_critical_sections = 0;
  carry_in = false;
  other_attr = 0;
  speedup = 1;
}

Task::Task(uint id, ResourceSet *resourceset, Param param, ulong wcet,
           ulong period, ulong deadline, uint priority) {
  this->id = id;
  this->index = id;
  this->wcet = wcet;
  if (0 == deadline)
    this->deadline = period;
  else
    this->deadline = deadline;
  this->period = period;
  this->priority = priority;
  utilization = this->wcet;
  utilization /= this->period;
  density = this->wcet;
  if (this->deadline <= this->period)
    density /= this->deadline;
  else
    density /= this->period;
  partition = MAX_INT;
  spin = 0;
  self_suspension = 0;
  local_blocking = 0;
  total_blocking = 0;
  jitter = 0;
  response_time = deadline;
  cluster = MAX_INT;
  independent = true;
  wcet_non_critical_sections = this->wcet;
  wcet_critical_sections = 0;
  carry_in = false;
  other_attr = 0;
  speedup = 1;

  uint critical_section_num = 0;

  for (int i = 0; i < param.p_num; i++) {
    if (i < param.ratio.size())
      ratio.push_back(param.ratio[i]);
    else
      ratio.push_back(1);
  }

  for (int i = 0; i < resourceset->size(); i++) {
    if ((param.mcsn > critical_section_num) && (param.rrr.min < wcet)) {
      if (Random_Gen::probability(param.rrp)) {
        uint num = Random_Gen::uniform_integral_gen(
            1, min(param.rrn,
                   static_cast<uint>(param.mcsn - critical_section_num)));
        uint max_len = Random_Gen::uniform_integral_gen(
            param.rrr.min, min(static_cast<double>(wcet), param.rrr.max));
        ulong length = num * max_len;
        if (length >= wcet_non_critical_sections) continue;

        wcet_non_critical_sections -= length;
        wcet_critical_sections += length;

        add_request(i, num, max_len, num * max_len,
                    resourceset->get_resources()[i].get_locality());

        resourceset->add_task(i, id);
        critical_section_num += num;
      }
    }
  }
}

Task::Task(	uint id,
			uint r_id,
			ResourceSet& resourceset,
			ulong ncs_wcet, 
			ulong cs_wcet, 
			ulong period,
			ulong deadline,
			uint priority) {
	this->id = id;
	this->index = id;
	this->wcet = ncs_wcet + cs_wcet;
	wcet_non_critical_sections = ncs_wcet;
	wcet_critical_sections = cs_wcet;
	if(0 == deadline)
		this->deadline = period;
	else
		this->deadline = deadline;
	this->period = period;
	this->priority = priority;
	utilization = this->wcet;
	utilization /= this->period;
	density = this->wcet;
	if (this->deadline <= this->period)
		density /= this->deadline;
	else
		density /= this->period;
	partition = MAX_INT;
	spin = 0;
	self_suspension = 0;
	local_blocking = 0;
	total_blocking = 0;
	jitter = 0;
	response_time = deadline;
	cluster = MAX_INT;
	independent = true;
	wcet_non_critical_sections = this->wcet;
	wcet_critical_sections = 0;
	carry_in = false;
	other_attr = 0;


	add_request(r_id, 1, cs_wcet, cs_wcet, resourceset.get_resources()[r_id].get_locality());
	resourceset.add_task(r_id, id);
}

Task::~Task() {
  // if(affinity)
  // delete affinity;
  requests.clear();
}

void Task::init() {
  partition = MAX_INT;
  spin = 0;
  self_suspension = 0;
  local_blocking = 0;
  total_blocking = 0;
  jitter = 0;
  response_time = deadline;
  cluster = MAX_INT;
  // priority = MAX_INT;
  independent = true;
  // wcet_non_critical_sections = this->wcet;
  // wcet_critical_sections = 0;
  carry_in = false;
  other_attr = 0;
  speedup = 1;
}

uint Task::task_model() { return model; }

void Task::add_request(uint res_id, uint num, ulong max_len, ulong total_len,
                       uint locality) {
  requests.push_back(Request(res_id, num, max_len, total_len, locality));
}

ulong Task::DBF(ulong time) {
  if (time >= deadline)
    return ((time - deadline) / period + 1) * wcet;
  else
    return 0;
}

uint Task::get_max_job_num(ulong interval) const {
  uint num_jobs;
  num_jobs = ceiling(interval + get_response_time(), get_period());
  return num_jobs;
}

uint Task::get_max_request_num(uint resource_id, ulong interval) const {
  if (is_request_exist(resource_id)) {
    const Request &request = get_request_by_id(resource_id);
    return get_max_job_num(interval) * request.get_num_requests();
  } else {
    return 0;
  }
}

fraction_t Task::get_utilization() const { return utilization; }

fraction_t Task::get_NCS_utilization() const {
  fraction_t temp = get_wcet_non_critical_sections();
  temp /= get_period();
  return temp;
}

fraction_t Task::get_density() const { return density; }

uint Task::get_id() const { return id; }
void Task::set_id(uint id) { this->id = id; }
uint Task::get_index() const { return index; }
void Task::set_index(uint index) { this->index = index; }
ulong Task::get_wcet() const { return wcet; }
ulong Task::get_wcet_heterogeneous() const {
  if (partition != 0XFFFFFFFF)
    return ceiling(wcet, speedup);
  else
    return wcet;
}
ulong Task::get_deadline() const { return deadline; }
ulong Task::get_period() const { return period; }
ulong Task::get_slack() const { return deadline - wcet; }
bool Task::is_feasible() const {
  return deadline >= wcet && period >= wcet && wcet > 0;
}

const Resource_Requests &Task::get_requests() const { return requests; }
const Request &Task::get_request_by_id(uint id) const {
  Request *result = NULL;
  for (uint i = 0; i < requests.size(); i++) {
    if (id == requests[i].get_resource_id()) return requests[i];
  }
  return *result;
}

bool Task::is_request_exist(uint resource_id) const {
  for (uint i = 0; i < requests.size(); i++) {
    if (resource_id == requests[i].get_resource_id()) return true;
  }
  return false;
}

void Task::update_requests(ResourceSet resources) {
  foreach(requests, request) {
    uint q = request->get_resource_id();
    request->set_locality(resources.get_resource_by_id(q).get_locality());
  }
}

ulong Task::get_wcet_critical_sections() const {
  return wcet_critical_sections;
}
ulong Task::get_wcet_critical_sections_heterogeneous() const {
  if (partition != 0XFFFFFFFF)
    return ceiling(wcet_critical_sections, speedup);
  else
    return wcet_critical_sections;
}
void Task::set_wcet_critical_sections(ulong csl) {
  wcet_critical_sections = csl;
}
ulong Task::get_wcet_non_critical_sections() const {
  return wcet_non_critical_sections;
}
ulong Task::get_wcet_non_critical_sections_heterogeneous() const {
  if (partition != 0XFFFFFFFF)
    return ceiling(wcet_non_critical_sections, speedup);
  else
    return wcet_non_critical_sections;
}
void Task::set_wcet_non_critical_sections(ulong ncsl) {
  wcet_non_critical_sections = ncsl;
}
ulong Task::get_spin() const { return spin; }
void Task::set_spin(ulong spining) { spin = spining; }
ulong Task::get_local_blocking() const { return local_blocking; }
void Task::set_local_blocking(ulong lb) { local_blocking = lb; }
ulong Task::get_remote_blocking() const { return remote_blocking; }
void Task::set_remote_blocking(ulong rb) { remote_blocking = rb; }
ulong Task::get_total_blocking() const { return total_blocking; }
void Task::set_total_blocking(ulong tb) { total_blocking = tb; }
ulong Task::get_self_suspension() const { return self_suspension; }
void Task::set_self_suspension(ulong ss) { self_suspension = ss; }
ulong Task::get_jitter() const { return jitter; }
void Task::set_jitter(ulong jit) { jitter = jit; }
ulong Task::get_response_time() const { return response_time; }
void Task::set_response_time(ulong response) { response_time = response; }
uint Task::get_priority() const { return priority; }
void Task::set_priority(uint prio) { priority = prio; }
uint Task::get_partition() const { return partition; }
void Task::set_partition(uint cpu, double speedup) {
  partition = cpu;
  this->speedup = speedup;
}
double Task::get_ratio(uint cpu_id) const { return ratio[cpu_id]; }
void Task::set_ratio(uint cpu_id, double speed) { ratio[cpu_id] = speed; }
uint Task::get_cluster() const { return cluster; }
void Task::set_cluster(uint clu) { cluster = clu; }
CPU_Set *Task::get_affinity() const { return affinity; }
void Task::set_affinity(CPU_Set *affi) { affinity = affi; }
bool Task::is_independent() const { return independent; }
void Task::set_dependent() { independent = false; }
bool Task::is_carry_in() const { return carry_in; }
void Task::set_carry_in() { carry_in = true; }
void Task::clear_carry_in() { carry_in = false; }
ulong Task::get_other_attr() const { return other_attr; }
void Task::set_other_attr(ulong attr) { other_attr = attr; }

/** Class TaskSet */

TaskSet::TaskSet() {
  utilization_sum = 0;
  utilization_max = 0;
  density_sum = 0;
  density_max = 0;
}

TaskSet::~TaskSet() { tasks.clear(); }

void TaskSet::init() {
  for (uint i = 0; i < tasks.size(); i++) tasks[i].init();
}

fraction_t TaskSet::get_utilization_sum() const { return utilization_sum; }

fraction_t TaskSet::get_utilization_max() const { return utilization_max; }

fraction_t TaskSet::get_density_sum() const { return density_sum; }

fraction_t TaskSet::get_density_max() const { return density_max; }


void TaskSet::add_task(Task task) {
  tasks.push_back(task);
}

void TaskSet::add_task(ulong wcet, ulong period, ulong deadline) {
  fraction_t utilization_new = wcet, density_new = wcet;
  utilization_new /= period;
  if (0 == deadline)
    density_new /= period;
  else
    density_new /= min(deadline, period);
  tasks.push_back(Task(tasks.size(), wcet, period, deadline));
  utilization_sum += utilization_new;
  density_sum += density_new;
  if (utilization_max < utilization_new) utilization_max = utilization_new;
  if (density_max < density_new) density_max = density_new;
}

void TaskSet::add_task(ResourceSet *resourceset, Param param, ulong wcet,
                       ulong period, ulong deadline) {
  fraction_t utilization_new = wcet, density_new = wcet;
  utilization_new /= period;
  if (0 == deadline)
    density_new /= period;
  else
    density_new /= min(deadline, period);
  tasks.push_back(
      Task(tasks.size(), resourceset, param, wcet, period, deadline));
  utilization_sum += utilization_new;
  density_sum += density_new;
  if (utilization_max < utilization_new) utilization_max = utilization_new;
  if (density_max < density_new) density_max = density_new;
}

void TaskSet::add_task(uint r_id, ResourceSet& resourceset, Param param, long ncs_wcet, long cs_wcet, long period, long deadline)
{
	
	fraction_t utilization_new = ncs_wcet + cs_wcet, density_new = ncs_wcet + cs_wcet;
	utilization_new /= period;
	if(0 == deadline)
		density_new /= period;
	else
		density_new /= min(deadline, period);
	tasks.push_back(Task(tasks.size(),
			r_id,
			resourceset,
			ncs_wcet, 
			cs_wcet, 
			period,
			deadline));

	utilization_sum += utilization_new;
	density_sum += density_new;
	if(utilization_max < utilization_new)
		utilization_max = utilization_new;
	if(density_max < density_new)
		density_max = density_new;
}

void TaskSet::calculate_spin(ResourceSet *resourceset,
                             ProcessorSet *processorset) {
  ulong spinning = 0;
  for (uint i = 0; i < tasks.size(); i++) {
    Task &task_i = tasks[i];
    // cout<<"request num:"<<task_i.get_requests().size()<<endl;
    for (uint j = 0; j < task_i.get_requests().size(); j++) {
      const Request &request = task_i.get_requests()[j];
      uint id = request.get_resource_id();
      uint num = request.get_num_requests();
      ulong Sum = 0;
      for (uint processor_id = 0;
           processor_id < processorset->get_processor_num(); processor_id++) {
        if (processor_id != task_i.get_partition()) {
          Processor &processor = processorset->get_processors()[processor_id];
          ulong max_length = 0;
          foreach(processor.get_taskqueue(), t_id) {
            Task &task_k = get_task_by_id(*t_id);
            if (task_k.is_request_exist(id)) {
              const Request &request_k = task_k.get_request_by_id(id);
              if (max_length < request_k.get_max_length())
                max_length = request_k.get_max_length();
            }
          }
          Sum += max_length;
        }
      }
      spinning += num * Sum;
    }
    task_i.set_spin(spinning);
  }
}

void TaskSet::calculate_local_blocking(ResourceSet *resourceset) {
  for (uint i = 0; i < tasks.size(); i++) {
    Task &task_i = tasks[i];
    ulong lb = 0;
    for (uint j = 0; j < task_i.get_requests().size(); j++) {
      const Request &request_i = task_i.get_requests()[j];
      uint id = request_i.get_resource_id();
      if (resourceset->get_resources()[id].get_ceiling() <= i) {
        for (uint k = task_i.get_id() + 1; k < tasks.size(); k++) {
          Task &task_k = tasks[k];
          const Request &request_k = task_k.get_request_by_id(id);
          if (&request_k) {
            lb = max(lb, request_k.get_max_length());
          }
        }
      }
    }
    task_i.set_local_blocking(lb);
  }
}

void TaskSet::calculate_spin_heterogeneous(ResourceSet *resourceset,
                                           ProcessorSet *processorset) {
  ulong spinning = 0;
  for (uint i = 0; i < tasks.size(); i++) {
    Task &task_i = tasks[i];
    // cout<<"request num:"<<task_i.get_requests().size()<<endl;
    for (uint j = 0; j < task_i.get_requests().size(); j++) {
      const Request &request = task_i.get_requests()[j];
      uint id = request.get_resource_id();
      uint num = request.get_num_requests();
      ulong Sum = 0;
      for (uint processor_id = 0;
           processor_id < processorset->get_processor_num(); processor_id++) {
        if (processor_id != task_i.get_partition()) {
          Processor &processor = processorset->get_processors()[processor_id];
          ulong max_length = 0;
          foreach(processor.get_taskqueue(), t_id) {
            Task &task_k = get_task_by_id(*t_id);
            if (task_k.is_request_exist(id)) {
              const Request &request_k = task_k.get_request_by_id(id);
              if (max_length < static_cast<double>(request_k.get_max_length()) /
                                   processor.get_speedfactor())
                max_length = static_cast<double>(request_k.get_max_length()) /
                             processor.get_speedfactor();
            }
          }
          Sum += max_length;
        }
      }
      spinning += num * Sum;
    }
    task_i.set_spin(spinning);
  }
}

void TaskSet::calculate_local_blocking_heterogeneous(
    ResourceSet *resourceset, ProcessorSet *processorset) {
  for (uint i = 0; i < tasks.size(); i++) {
    Task &task_i = tasks[i];
    uint p_id = task_i.get_partition();
    if (MAX_INT == p_id) continue;
    Processor &processor = processorset->get_processors()[p_id];
    ulong lb = 0;
    for (uint j = 0; j < task_i.get_requests().size(); j++) {
      const Request &request_i = task_i.get_requests()[j];
      uint id = request_i.get_resource_id();
      if (resourceset->get_resources()[id].get_ceiling() <= i) {
        for (uint k = task_i.get_id() + 1; k < tasks.size(); k++) {
          Task &task_k = tasks[k];
          const Request &request_k = task_k.get_request_by_id(id);
          if (&request_k) {
            lb = max(static_cast<double>(lb),
                     static_cast<double>(request_k.get_max_length()) /
                         processor.get_speedfactor());
          }
        }
      }
    }

    task_i.set_local_blocking(lb);
  }
}

Tasks &TaskSet::get_tasks() { return tasks; }

Task &TaskSet::get_task_by_id(uint id) {
  foreach(tasks, task) {
    if (id == task->get_id()) return (*task);
  }
  return *reinterpret_cast<Task *>(0);
}

Task &TaskSet::get_task_by_index(uint index) { return tasks[index]; }

Task &TaskSet::get_task_by_priority(uint pi) {
  foreach(tasks, task) {
    if (pi == task->get_priority()) return (*task);
  }
  return *reinterpret_cast<Task *>(0);
}

bool TaskSet::is_implicit_deadline() {
  foreach_condition(tasks, tasks[i].get_deadline() != tasks[i].get_period());
  return true;
}
bool TaskSet::is_constrained_deadline() {
  foreach_condition(tasks, tasks[i].get_deadline() > tasks[i].get_period());
  return true;
}
bool TaskSet::is_arbitary_deadline() {
  return !(is_implicit_deadline()) && !(is_constrained_deadline());
}
uint TaskSet::get_taskset_size() const { return tasks.size(); }

fraction_t TaskSet::get_task_utilization(uint index) const {
  return tasks[index].get_utilization();
}
fraction_t TaskSet::get_task_density(uint index) const {
  return tasks[index].get_density();
}
ulong TaskSet::get_task_wcet(uint index) const {
  return tasks[index].get_wcet();
}
ulong TaskSet::get_task_deadline(uint index) const {
  return tasks[index].get_deadline();
}
ulong TaskSet::get_task_period(uint index) const {
  return tasks[index].get_period();
}

void TaskSet::sort_by_id() {
  sort(tasks.begin(), tasks.end(), id_increase<Task>);
}

void TaskSet::sort_by_index() {
  sort(tasks.begin(), tasks.end(), index_increase<Task>);
}

void TaskSet::sort_by_period() {
  sort(tasks.begin(), tasks.end(), period_increase<Task>);
  for (int i = 0; i < tasks.size(); i++) tasks[i].set_index(i);
}

void TaskSet::sort_by_deadline() {
  sort(tasks.begin(), tasks.end(), deadline_increase<Task>);
  for (int i = 0; i < tasks.size(); i++) tasks[i].set_index(i);
}

void TaskSet::sort_by_utilization() {
  sort(tasks.begin(), tasks.end(), utilization_decrease<Task>);
  for (int i = 0; i < tasks.size(); i++) tasks[i].set_index(i);
}

void TaskSet::sort_by_density() {
  sort(tasks.begin(), tasks.end(), density_decrease<Task>);
  for (int i = 0; i < tasks.size(); i++) tasks[i].set_index(i);
}

void TaskSet::sort_by_DC() {
  sort(tasks.begin(), tasks.end(), task_DC_increase<Task>);
  for (int i = 0; i < tasks.size(); i++) tasks[i].set_index(i);
}

void TaskSet::sort_by_DCC() {
  sort(tasks.begin(), tasks.end(), task_DCC_increase<Task>);
  for (int i = 0; i < tasks.size(); i++) tasks[i].set_index(i);
}

void TaskSet::sort_by_DDC() {
  sort(tasks.begin(), tasks.end(), task_DDC_increase<Task>);
  for (int i = 0; i < tasks.size(); i++) tasks[i].set_index(i);
}

void TaskSet::sort_by_UDC() {
  sort(tasks.begin(), tasks.end(), task_UDC_increase<Task>);
  for (int i = 0; i < tasks.size(); i++) tasks[i].set_index(i);
}

void TaskSet::RM_Order() {
  sort_by_period();
  for (uint i = 0; i < tasks.size(); i++) {
    tasks[i].set_priority(i);
  }

#if SORT_DEBUG
  cout << "RM Order:" << endl;
  cout << "-----------------------" << endl;
  foreach(tasks, task) {
    cout << " Task " << task->get_id() << ":" << endl;
    cout << " WCET:" << task->get_wcet() << " Deadline:" << task->get_deadline()
         << " Period:" << task->get_period()
         << " Gap:" << task->get_deadline() - task->get_wcet()
         << " Leisure:" << leisure(task->get_id()) << endl;
    cout << "-----------------------" << endl;
  }
#endif
}

void TaskSet::DM_Order() {
  sort_by_deadline();
  for (uint i = 0; i < tasks.size(); i++) {
    tasks[i].set_priority(i);
  }
}

void TaskSet::Density_Decrease_Order() {
  sort_by_density();
  for (uint i = 0; i < tasks.size(); i++) {
    tasks[i].set_priority(i);
  }
}

void TaskSet::DC_Order() {
  sort_by_DC();
  for (uint i = 0; i < tasks.size(); i++) {
    tasks[i].set_priority(i);
  }

#if SORT_DEBUG
  cout << "DC Order:" << endl;
  cout << "-----------------------" << endl;
  foreach(tasks, task) {
    cout << " Task " << task->get_id() << ":" << endl;
    cout << " WCET:" << task->get_wcet() << " Deadline:" << task->get_deadline()
         << " Period:" << task->get_period()
         << " Gap:" << task->get_deadline() - task->get_wcet()
         << " Leisure:" << leisure(task->get_id()) << endl;
    cout << "-----------------------" << endl;
  }
#endif
}

void TaskSet::DCC_Order() {
  sort_by_DCC();
  for (uint i = 0; i < tasks.size(); i++) {
    tasks[i].set_priority(i);
  }
}

void TaskSet::DDC_Order() {
  sort_by_DDC();
  for (uint i = 0; i < tasks.size(); i++) {
    tasks[i].set_priority(i);
  }
}

void TaskSet::UDC_Order() {
  sort_by_UDC();
  for (uint i = 0; i < tasks.size(); i++) {
    tasks[i].set_priority(i);
  }
}

void TaskSet::SM_PLUS_Order() {
  sort_by_DC();

  if (1 < tasks.size()) {
    vector<ulong> accum;
    for (uint i = 0; i < tasks.size(); i++) {
      accum.push_back(0);
    }
    for (int index = 0; index < tasks.size() - 1; index++) {
      vector<Task>::iterator it = (tasks.begin() + index);
      ulong c = (it)->get_wcet();
      ulong gap = (it)->get_deadline() - (it)->get_wcet();
      ulong d = (it)->get_deadline();

      for (int index2 = index + 1; index2 < tasks.size(); index2++) {
        vector<Task>::iterator it2 = (tasks.begin() + index2);
        ulong c2 = (it2)->get_wcet();
        ulong gap2 = (it2)->get_deadline() - (it2)->get_wcet();
        ulong d2 = (it2)->get_deadline();

        if (c > gap2 && d > d2) {
          accum[(it->get_id())] += c2;

          Task temp = (*it2);
          tasks.erase((it2));
          tasks.insert(it, temp);

          index = 0;
          break;
        }
      }
    }

    for (int i = 0; i < tasks.size(); i++) tasks[i].set_index(i);

  }

  for (uint i = 0; i < tasks.size(); i++) {
    tasks[i].set_priority(i);
  }
}

void TaskSet::SM_PLUS_2_Order() {
  sort_by_DC();

  if (1 < tasks.size()) {
    vector<ulong> accum;
    for (uint i = 0; i < tasks.size(); i++) {
      accum.push_back(0);
    }
    for (int index = 0; index < tasks.size() - 1; index++) {
      vector<Task>::iterator it = (tasks.begin() + index);
      ulong c = (it)->get_wcet();
      ulong gap = (it)->get_deadline() - (it)->get_wcet();
      ulong d = (it)->get_deadline();

      for (int index2 = index + 1; index2 < tasks.size(); index2++) {
        vector<Task>::iterator it2 = (tasks.begin() + index2);
        ulong c2 = (it2)->get_wcet();
        ulong gap2 = (it2)->get_deadline() - (it2)->get_wcet();
        ulong d2 = (it2)->get_deadline();

        if (c > gap2 && d > d2 && (accum[(it->get_id())] + c2) <= gap) {
          accum[(it->get_id())] += c2;

          Task temp = (*it2);
          tasks.erase((it2));
          tasks.insert(it, temp);

          index = 0;
          break;
        }
      }
    }

    for (int i = 0; i < tasks.size(); i++) tasks[i].set_index(i);
  }

  for (uint i = 0; i < tasks.size(); i++) {
    tasks[i].set_priority(i);
  }

#if SORT_DEBUG
  cout << "SMP2:" << endl;
  cout << "-----------------------" << endl;
  foreach(tasks, task) {
    cout << " Task " << task->get_id() << ":" << endl;
    cout << " WCET:" << task->get_wcet() << " Deadline:" << task->get_deadline()
         << " Period:" << task->get_period()
         << " Gap:" << task->get_deadline() - task->get_wcet()
         << " Leisure:" << leisure(task->get_id()) << endl;
    cout << "-----------------------" << endl;
  }
#endif
}

void TaskSet::SM_PLUS_3_Order() {
  sort_by_period();

  if (1 < tasks.size()) {
    vector<ulong> accum;
    for (uint i = 0; i < tasks.size(); i++) {
      accum.push_back(0);
    }
    for (int index = 0; index < tasks.size() - 1; index++) {
      vector<Task>::iterator it = (tasks.begin() + index);
      ulong c = (it)->get_wcet();
      ulong gap = (it)->get_deadline() - (it)->get_wcet();
      ulong d = (it)->get_deadline();
      ulong p = (it)->get_period();

      for (int index2 = index + 1; index2 < tasks.size(); index2++) {
        vector<Task>::iterator it2 = (tasks.begin() + index2);
        ulong c2 = (it2)->get_wcet();
        ulong gap2 = (it2)->get_deadline() - (it2)->get_wcet();
        ulong d2 = (it2)->get_deadline();
        ulong p2 = (it2)->get_period();
        uint N = ceiling(p, p2);
        if (gap > gap2 && (accum[(it->get_id())] + N * c2) <= gap) {
          accum[(it->get_id())] += N * c2;

          Task temp = (*it2);
          tasks.erase((it2));
          tasks.insert(it, temp);

          index = 0;
          break;
        }
      }
    }

    for (int i = 0; i < tasks.size(); i++) tasks[i].set_index(i);
  }

  for (uint i = 0; i < tasks.size(); i++) {
    tasks[i].set_priority(i);
  }

#if SORT_DEBUG
  cout << "SMP3:" << endl;
  cout << "-----------------------" << endl;
  foreach(tasks, task) {
    cout << " Task " << task->get_id() << ":" << endl;
    cout << " WCET:" << task->get_wcet() << " Deadline:" << task->get_deadline()
         << " Period:" << task->get_period()
         << " Gap:" << task->get_deadline() - task->get_wcet()
         << " Leisure:" << leisure(task->get_id()) << endl;
    cout << "-----------------------" << endl;
  }
#endif
}

void TaskSet::Leisure_Order() {
  Tasks NewSet;
  sort_by_id();

  for (int i = tasks.size() - 1; i >= 0; i--) {
    int64_t l_max = 0xffffffffffffffff;
    uint index = i;
    for (int j = i; j >= 0; j--) {
      vector<Task>::iterator it = (tasks.begin() + j);
      Task temp = (*it);
      tasks.erase((it));
      tasks.push_back(temp);
      int64_t l = leisure(i);
      if (l > l_max) {
        l_max = l;
        index = j;
      }
    }
    sort_by_id();
    vector<Task>::iterator it2 = (tasks.begin() + index);
    Task temp2 = (*it2);
    tasks.erase(it2);
    NewSet.insert(NewSet.begin(), temp2);
  }

  tasks.clear();
  tasks = NewSet;

  for (uint i = 0; i < tasks.size(); i++) {
    tasks[i].set_index(i);
    tasks[i].set_priority(i);
  }

  cout << "Leisure Order:" << endl;
  cout << "-----------------------" << endl;
  foreach(tasks, task) {
    cout << " Task " << task->get_id() << ":" << endl;
    cout << " WCET:" << task->get_wcet() << " Deadline:" << task->get_deadline()
         << " Period:" << task->get_period()
         << " Gap:" << task->get_deadline() - task->get_wcet()
         << " Leisure:" << leisure(task->get_id()) << endl;
    cout << "-----------------------" << endl;
  }
}

void TaskSet::SM_PLUS_4_Order(uint p_num) {
  sort_by_period();
#if SORT_DEBUG
  cout << "////////////////////////////////////////////" << endl;
  cout << "RM:" << endl;
  cout << "-----------------------" << endl;
  foreach(tasks, task) {
    cout << " Task " << task->get_id() << ":" << endl;
    cout << " WCET:" << task->get_wcet() << " Deadline:" << task->get_deadline()
         << " Period:" << task->get_period()
         << " Gap:" << task->get_deadline() - task->get_wcet()
         << " Leisure:" << leisure(task->get_id()) << endl;
    cout << "-----------------------" << endl;
  }
#endif

  if (1 < tasks.size()) {
    uint min_id;
    ulong min_slack;
    vector<uint> id_stack;

    foreach(tasks, task) {
      task->set_other_attr(0);  // accumulative adjustment
    }

    for (uint index = 0; index < tasks.size(); index++) {
      bool is_continue = false;
      min_id = 0;
      min_slack = MAX_LONG;
      foreach(tasks, task) {  // find next minimum slack
        is_continue = false;
        uint temp_id = task->get_id();
        foreach(id_stack, element) {
          if (temp_id == (*element)) is_continue = true;
        }
        if (is_continue) continue;
        ulong temp_slack = task->get_slack();
        if (min_slack > task->get_slack()) {
          min_id = temp_id;
          min_slack = temp_slack;
        }
      }
      id_stack.push_back(min_id);
      is_continue = false;

      // locate minimum slack
      for (uint index2 = 0; index2 < tasks.size(); index2++) {
        if (min_id == tasks[index2].get_id()) {
          vector<Task>::iterator task1 = (tasks.begin() + index2);
          for (int index3 = index2 - 1; index3 >= 0; index3--) {
            vector<Task>::iterator task2 = (tasks.begin() + index3);
            if ((p_num - 1) <= task2->get_other_attr()) {
              is_continue = true;
              break;
            }
            if (task1->get_slack() < task2->get_slack()) {
              Task temp = (*task1);
              tasks.erase((task1));
              if (task2->get_deadline() < task1->get_wcet() + task2->get_wcet())
                task2->set_other_attr(task2->get_other_attr() + 1);
              tasks.insert(task2, temp);
              task1 = (tasks.begin() + index3);
            }
          }

          break;
        }
      }
    }

    for (int i = 0; i < tasks.size(); i++) tasks[i].set_index(i);
  }

  for (uint i = 0; i < tasks.size(); i++) {
    tasks[i].set_priority(i);
  }

#if SORT_DEBUG
  cout << "SMP4:" << endl;
  cout << "-----------------------" << endl;
  foreach(tasks, task) {
    cout << " Task " << task->get_id() << ":" << endl;
    cout << " WCET:" << task->get_wcet() << " Deadline:" << task->get_deadline()
         << " Period:" << task->get_period()
         << " Gap:" << task->get_deadline() - task->get_wcet()
         << " Leisure:" << leisure(task->get_id()) << endl;
    cout << "-----------------------" << endl;
  }
#endif
}

ulong TaskSet::leisure(uint index) {
  Task &task = get_task_by_id(index);
  ulong gap = task.get_deadline() - task.get_wcet();
  ulong period = task.get_period();
  ulong remain = gap;
  for (uint i = 0; i < index; i++) {
    Task &temp = get_task_by_id(i);
    remain -= temp.get_wcet() * ceiling(period, temp.get_period());
  }
  return remain;
}

void TaskSet::display() {
  for (int i = 0; i < tasks.size(); i++) {
    cout << "Task index:" << tasks[i].get_index()
         << " Task id:" << tasks[i].get_id()
         << " Task priority:" << tasks[i].get_priority() << endl;
  }
}

void TaskSet::update_requests(const ResourceSet& resources) {
  foreach(tasks, task) {
    task->update_requests(resources);
  }
}

void TaskSet::export_taskset(const char *path) {
  XML output;

  output.initialization();

  output.add_element("taskset");

  output.add_element("taskset", "utilization",
                     to_string(get_utilization_sum().get_d()).data());

  for (uint i = 0; i < tasks.size(); i++) {
    output.add_element("taskset", "task");
    XMLElement *ts = output.get_element("taskset");
    XMLElement *t = output.get_element(ts, "task", i);

    output.add_element(t, "wcet", to_string(tasks[i].get_wcet()).data());

    output.add_element(
        t, "ncs-wcet",
        to_string(tasks[i].get_wcet_non_critical_sections()).data());

    output.add_element(t, "cs-wcet",
                       to_string(tasks[i].get_wcet_critical_sections()).data());

    output.add_element(t, "deadline",
                       to_string(tasks[i].get_deadline()).data());

    output.add_element(t, "period", to_string(tasks[i].get_period()).data());

    output.add_element(t, "u",
                       to_string(tasks[i].get_utilization().get_d()).data());

    if (0 != tasks[i].get_requests().size()) {
      output.add_element(t, "request");
      XMLElement *req = output.get_element(t, "request");

      for (uint j = 0; j < tasks[i].get_requests().size(); j++) {
        output.add_element(req, "resource");
        XMLElement *res = output.get_element(req, "resource", j);

        output.add_element(
            res, "id",
            to_string(tasks[i].get_requests()[j].get_resource_id()).data());

        output.add_element(
            res, "N",
            to_string(tasks[i].get_requests()[j].get_num_requests()).data());

        output.add_element(
            res, "L",
            to_string(tasks[i].get_requests()[j].get_max_length()).data());
      }
    }
  }

  output.save_file(path);
}

/** Task DAG_Task */

DAG_Task::DAG_Task(const DAG_Task &dt)
    : Task(dt.get_id(), 0, dt.get_period(), dt.get_deadline(),
           dt.get_priority()) {
  task_id = dt.task_id;
  vnodes = dt.vnodes;
  arcnodes = dt.arcnodes;
  vol = dt.vol;  // total wcet of the jobs in graph
  len = dt.len;
  deadline = dt.deadline;
  period = dt.period;
  utilization = dt.utilization;
  density = dt.density;
  vexnum = dt.vexnum;
  arcnum = dt.arcnum;
  spin = dt.spin;
  self_suspension = dt.self_suspension;
  local_blocking = dt.local_blocking;
  remote_blocking = dt.remote_blocking;
  total_blocking = dt.total_blocking;
  jitter = dt.jitter;
  response_time = dt.response_time;  // initialization as WCET
  priority = dt.priority;
  partition = dt.partition;  // 0XFFFFFFFF
  ratio = dt.ratio;          // for heterogeneous platform
  requests = dt.requests;

  refresh_relationship();
}

DAG_Task::DAG_Task(uint task_id, ulong period, ulong deadline, uint priority)
    : Task(task_id, 0, period, deadline, priority) {
  len = 0;
  vol = 0;
  if (0 == deadline) this->deadline = period;
  this->period = period;
  vexnum = 0;
  arcnum = 0;
  spin = 0;
  self_suspension = 0;
  local_blocking = 0;
  remote_blocking = 0;
  total_blocking = 0;
  jitter = 0;
  response_time = 0;
  priority = priority;
  partition = 0XFFFFFFFF;
  utilization = 0;
  density = 0;
  refresh_relationship();
}

DAG_Task::DAG_Task(uint task_id, ResourceSet *resourceset, Param param,
                   ulong vol, ulong period, ulong deadline, uint priority)
    : Task(task_id, resourceset, param, vol, period, deadline, priority) {
  this->task_id = task_id;
  this->len = 0;
  this->vol = vol;
  if (0 == deadline)
    this->deadline = period;
  else
    this->deadline = deadline;
  this->period = period;
  vexnum = 0;
  arcnum = 0;
  spin = 0;
  self_suspension = 0;
  local_blocking = 0;
  remote_blocking = 0;
  total_blocking = 0;
  jitter = 0;
  response_time = 0;
  priority = 0;
  partition = 0XFFFFFFFF;
  utilization = vol;
  utilization /= period;
  density = 0;

  // Generate basic graph
  uint JobNode_num = Random_Gen::uniform_integral_gen(
      static_cast<int>(max(1, static_cast<int>(param.job_num_range.min))),
      static_cast<int>(param.job_num_range.max));
  if (JobNode_num > vol) JobNode_num = vol;
  graph_gen(&vnodes, &arcnodes, param, JobNode_num);

  // Insert conditional branches
  for (uint i = 1; i < vnodes.size() - 1; i++) {
    if (Random_Gen::probability(param.cond_prob)) {
      uint cond_job_num = Random_Gen::uniform_integral_gen(
          2, max(2, static_cast<int>(param.max_cond_branch)));
      vector<VNode> v;
      vector<ArcNode> a;
      sub_graph_gen(&v, &a, cond_job_num, G_TYPE_C);
      graph_insert(&v, &a, i);
      break;
    }
  }

  // Alocate wcet
  uint job_node_num = 0;
  for (uint i = 0; i < vnodes.size(); i++) {
    if (J_NODE == vnodes[i].type) job_node_num++;
  }
  vector<ulong> wcets;
  wcets.push_back(0);
  for (uint i = 1; i < job_node_num; i++) {
    bool test;
    ulong temp;
    do {
      test = false;
      temp = Random_Gen::uniform_ulong_gen(1, vol - 1);
      for (uint j = 0; j < wcets.size(); j++)
        if (temp == wcets[j]) test = true;
    } while (test);
    wcets.push_back(temp);
  }
  wcets.push_back(vol);
  sort(wcets.begin(), wcets.end());
  for (uint i = 0, j = 0; i < vnodes.size(); i++) {
    if (J_NODE == vnodes[i].type) {
      vnodes[i].wcet = wcets[j + 1] - wcets[j];
      j++;
    }
    vnodes[i].deadline = deadline;
  }
  // display_vertices();
  // display_arcs();
  refresh_relationship();
  update_vol();
  update_len();
  density = len;
  density /= deadline;
}

void DAG_Task::graph_gen(vector<VNode> *v, vector<ArcNode> *a, Param param,
                         uint n_num, double arc_density) {
  v->clear();
  a->clear();
  // creating vnodes
  VNode polar_start, polar_end;
  polar_start.job_id = 0;
  polar_end.job_id = n_num + 1;
  polar_start.type = P_NODE | S_NODE;
  polar_end.type = P_NODE | E_NODE;
  polar_start.pair = polar_end.job_id;
  polar_end.pair = polar_start.job_id;
  polar_start.wcet = 0;
  polar_end.wcet = 0;

  v->push_back(polar_start);
  for (uint i = 0; i < n_num; i++) {
    VNode temp_node;
    temp_node.job_id = v->size();
    temp_node.type = J_NODE;
    temp_node.pair = MAX_INT;
    temp_node.wcet = 0;
    v->push_back(temp_node);
  }
  v->push_back(polar_end);
  // creating arcs
  uint ArcNode_num;
  if (param.is_cyclic)  // cyclic graph
    ArcNode_num = Random_Gen::uniform_integral_gen(0, n_num * (n_num - 1));
  else  // acyclic graph
    ArcNode_num =
        Random_Gen::uniform_integral_gen(0, (n_num * (n_num - 1)) / 2);
  for (uint i = 0; i < ArcNode_num; i++) {
    uint tail, head, temp;
    do {
      if (param.is_cyclic) {  // cyclic graph
        tail = Random_Gen::uniform_integral_gen(1, n_num);
        head = Random_Gen::uniform_integral_gen(1, n_num);
        if (tail == head) continue;
      } else {  // acyclic graph
        tail = Random_Gen::uniform_integral_gen(1, n_num);
        head = Random_Gen::uniform_integral_gen(1, n_num);
        if (tail == head) {
          continue;
        } else if (tail > head) {
          temp = tail;
          tail = head;
          head = temp;
        }
      }

      if (is_arc_exist(*a, tail, head)) {
        continue;
      }
      break;
    } while (true);
    add_arc(a, tail, head);
  }
  refresh_relationship(v, a);
  for (uint i = 1; i <= n_num; i++) {
    if (0 == (*v)[i].precedences.size()) add_arc(a, 0, i);
    if (0 == (*v)[i].follow_ups.size()) add_arc(a, i, n_num + 1);
  }
  refresh_relationship(v, a);
}

void DAG_Task::sub_graph_gen(vector<VNode> *v, vector<ArcNode> *a, uint n_num,
                             int G_TYPE) {
  v->clear();
  a->clear();

  // creating vnodes
  VNode polar_start, polar_end;
  polar_start.job_id = 0;
  polar_end.job_id = n_num + 1;
  if (G_TYPE == G_TYPE_P) {
    polar_start.type = P_NODE | S_NODE;
    polar_end.type = P_NODE | E_NODE;
  } else {
    polar_start.type = C_NODE | S_NODE;
    polar_end.type = C_NODE | E_NODE;
  }
  polar_start.pair = polar_end.job_id;
  polar_end.pair = polar_start.job_id;
  polar_start.wcet = 0;
  polar_end.wcet = 0;

  v->push_back(polar_start);
  for (uint i = 0; i < n_num; i++) {
    VNode temp_node;
    temp_node.job_id = v->size();
    temp_node.type = J_NODE;
    temp_node.pair = MAX_INT;
    temp_node.wcet = 0;
    v->push_back(temp_node);
  }
  v->push_back(polar_end);
  // creating arcs
  for (uint i = 1; i <= n_num; i++) {
    if (0 == (*v)[i].precedences.size()) add_arc(a, 0, i);
    if (0 == (*v)[i].follow_ups.size()) add_arc(a, i, n_num + 1);
  }
  refresh_relationship(v, a);
}

void DAG_Task::sequential_graph_gen(vector<VNode> *v, vector<ArcNode> *a,
                                    uint n_num) {
  v->clear();
  a->clear();
  for (uint i = 0; i < n_num; i++) {
    VNode temp_node;
    temp_node.job_id = v->size();
    temp_node.type = J_NODE;
    temp_node.pair = MAX_INT;
    temp_node.wcet = 0;
    v->push_back(temp_node);
  }
  for (uint i = 0; i < n_num - 1; i++) {
    add_arc(a, i, i + 1);
  }
  refresh_relationship(v, a);
}

void DAG_Task::graph_insert(vector<VNode> *v, vector<ArcNode> *a,
                            uint replace_node) {
  if (replace_node >= vnodes.size()) {
    cout << "Out of bound." << endl;
    return;
  }
  if (J_NODE != vnodes[replace_node].type) {
    cout << "Only job node could be replaced." << endl;
    return;
  }
  uint v_num = v->size();
  uint a_num = a->size();
  for (uint i = 0; i < v_num; i++) {
    (*v)[i].job_id += replace_node;
    cout << (*v)[i].job_id << endl;
    if (MAX_INT != (*v)[i].pair) (*v)[i].pair += replace_node;
  }
  int gap = v_num - 1;
  for (uint i = replace_node + 1; i < vnodes.size(); i++) {
    vnodes[i].job_id += gap;
  }
  for (uint i = 0; i < vnodes.size(); i++) {
    if ((vnodes[i].pair > replace_node) && (MAX_INT != vnodes[i].pair))
      vnodes[i].pair += gap;
  }
  for (uint i = 0; i < arcnodes.size(); i++) {
    if (arcnodes[i].tail >= replace_node) arcnodes[i].tail += gap;
    if (arcnodes[i].head > replace_node) arcnodes[i].head += gap;
  }
  vnodes.insert(vnodes.begin() + replace_node + 1, v->begin(), v->end());
  vnodes.erase(vnodes.begin() + replace_node);
  vector<ArcNode>::iterator it2 = arcnodes.begin();
  for (uint i = 0; i < a_num; i++) {
    (*a)[i].tail += replace_node;
    (*a)[i].head += replace_node;
  }
  for (vector<ArcNode>::iterator it = a->begin(); it < a->end(); it++)
    arcnodes.push_back(*it);

  refresh_relationship();
}

uint DAG_Task::task_model() { return model; }

uint DAG_Task::get_id() const { return task_id; }
void DAG_Task::set_id(uint id) { task_id = id; }
uint DAG_Task::get_vnode_num() const { return vnodes.size(); }
uint DAG_Task::get_arcnode_num() const { return arcnodes.size(); }
vector<VNode> &DAG_Task::get_vnodes() { return vnodes; }
VNode &DAG_Task::get_vnode_by_id(uint job_id) { return vnodes[job_id]; }
ulong DAG_Task::get_deadline() const { return deadline; }
ulong DAG_Task::get_period() const { return period; }
ulong DAG_Task::get_vol() const { return vol; }
ulong DAG_Task::get_len() const { return len; }
fraction_t DAG_Task::get_utilization() const { return utilization; }
fraction_t DAG_Task::get_density() const { return density; }

void DAG_Task::add_job(ulong wcet, ulong deadline) {
  VNode vnode;
  vnode.job_id = vnodes.size();
  vnode.wcet = wcet;
  if (0 == deadline) vnode.deadline = this->deadline;
  vnode.level = 0;
  vnodes.push_back(vnode);
  // update_vol();
  // update_len();
}

void DAG_Task::add_job(vector<VNode> *v, ulong wcet, ulong deadline) {
  VNode vnode;
  vnode.job_id = v->size();
  vnode.wcet = wcet;
  if (0 == deadline) vnode.deadline = this->deadline;
  vnode.level = 0;
  v->push_back(vnode);
}

void DAG_Task::add_arc(uint tail, uint head) {
  ArcNode arcnode;
  arcnode.tail = tail;
  arcnode.head = head;
  arcnodes.push_back(arcnode);
}

void DAG_Task::add_arc(vector<ArcNode> *a, uint tail, uint head) {
  ArcNode arcnode;
  arcnode.tail = tail;
  arcnode.head = head;
  a->push_back(arcnode);
}

void DAG_Task::delete_arc(uint tail, uint head) {}

void DAG_Task::refresh_relationship() {
  std::sort(arcnodes.begin(), arcnodes.end(), arcs_increase<ArcNode>);
  for (uint i = 0; i < vnodes.size(); i++) {
    vnodes[i].precedences.clear();
    vnodes[i].follow_ups.clear();
  }
  for (uint i = 0; i < arcnodes.size(); i++) {
    vnodes[arcnodes[i].tail].follow_ups.push_back(&arcnodes[i]);
    vnodes[arcnodes[i].head].precedences.push_back(&arcnodes[i]);
  }
}

void DAG_Task::refresh_relationship(vector<VNode> *v, vector<ArcNode> *a) {
  sort(a->begin(), a->end(), arcs_increase<ArcNode>);
  for (uint i = 0; i < v->size(); i++) {
    (*v)[i].precedences.clear();
    (*v)[i].follow_ups.clear();
  }
  for (uint i = 0; i < a->size(); i++) {
    (*v)[(*a)[i].tail].follow_ups.push_back(&(*a)[i]);
    (*v)[(*a)[i].head].precedences.push_back(&(*a)[i]);
  }
}

void DAG_Task::update_vol() {
  vol = 0;
  for (uint i = 0; i < vnodes.size(); i++) vol += vnodes[i].wcet;
}

void DAG_Task::update_len() {
  len = 0;
  for (uint i = 0; i < vnodes.size(); i++) {
    if (0 == vnodes[i].precedences.size()) {  // finding the head
      ulong temp = 0;
      for (uint j = 0; j < vnodes[i].follow_ups.size(); j++) {
        ulong temp2 =
            vnodes[i].wcet + DFS(vnodes[vnodes[i].follow_ups[j]->head]);
        if (temp < temp2) temp = temp2;
      }
      if (len < temp) len = temp;
    }
  }
}

bool DAG_Task::is_acyclic() { VNodePtr job = &vnodes[0]; }

ulong DAG_Task::DFS(VNode vnode) {
  ulong result = 0;
  if (0 == vnode.follow_ups.size()) {
    result = vnode.wcet;
  } else {
    for (uint i = 0; i < vnode.follow_ups.size(); i++) {
      ulong temp = vnode.wcet + DFS(vnodes[vnode.follow_ups[i]->head]);
      if (result < temp) result = temp;
    }
  }
  return result;
}

ulong DAG_Task::BFS(VNode vnode) {}

bool DAG_Task::is_arc_exist(uint tail, uint head) {
  for (uint i = 0; i < arcnodes.size(); i++) {
    if (tail == arcnodes[i].tail)
      if (head == arcnodes[i].head) return true;
  }
  return false;
}

bool DAG_Task::is_arc_exist(const vector<ArcNode> &a, uint tail, uint head) {
  for (uint i = 0; i < a.size(); i++) {
    if (tail == a[i].tail)
      if (head == a[i].head) return true;
  }
  return false;
}

void DAG_Task::display_vertices() {
  cout << "display main vertices:" << endl;
  for (uint i = 0; i < vnodes.size(); i++) {
    cout << vnodes[i].job_id << ":" << vnodes[i].wcet << ":" << vnodes[i].type;
    if (MAX_INT == vnodes[i].pair)
      cout << endl;
    else
      cout << ":" << vnodes[i].pair << endl;
  }
}

void DAG_Task::display_vertices(const vector<VNode> &v) {
  cout << "display vertices:" << endl;
  for (uint i = 0; i < v.size(); i++) {
    cout << v[i].job_id << ":" << v[i].type;
    if (MAX_INT == v[i].pair)
      cout << endl;
    else
      cout << ":" << v[i].pair << endl;
  }
}

void DAG_Task::display_arcs() {
  cout << "display main arcs:" << endl;
  for (uint i = 0; i < arcnodes.size(); i++) {
    cout << arcnodes[i].tail << "--->" << arcnodes[i].head
         << "\taddress:" << &arcnodes[i] << endl;
  }
}

void DAG_Task::display_arcs(const vector<ArcNode> &a) {
  cout << "display arcs:" << endl;
  for (uint i = 0; i < a.size(); i++) {
    cout << a[i].tail << "--->" << a[i].head << endl;
  }
}

void DAG_Task::display_follow_ups(uint job_id) {
  for (uint i = 0; i < vnodes[job_id].follow_ups.size(); i++) {
    cout << "follow up of node " << job_id << ":"
         << vnodes[job_id].follow_ups[i]->head
         << "\taddress:" << vnodes[job_id].follow_ups[i] << endl;
  }
}

void DAG_Task::display_precedences(uint job_id) {
  for (uint i = 0; i < vnodes[job_id].precedences.size(); i++)
    cout << "precedences of node " << job_id << ":"
         << vnodes[job_id].precedences[i]->tail
         << "\taddress:" << vnodes[job_id].precedences[i] << endl;
}

uint DAG_Task::get_indegrees(uint job_id) const {
  return vnodes[job_id].precedences.size();
}
uint DAG_Task::get_outdegrees(uint job_id) const {
  return vnodes[job_id].follow_ups.size();
}

/** Task DAG_TaskSet */

DAG_TaskSet::DAG_TaskSet() {
  utilization_sum = 0;
  utilization_max = 0;
  density_sum = 0;
  density_max = 0;
}

void DAG_TaskSet::add_task(ResourceSet *resourceset, Param param, ulong wcet,
                           ulong period, ulong deadline) {
  uint task_id = dag_tasks.size();
  dag_tasks.push_back(
      DAG_Task(task_id, resourceset, param, wcet, period, deadline));
  utilization_sum += get_task_by_id(task_id).get_utilization();
  density_sum += get_task_by_id(task_id).get_density();
  if (utilization_max < get_task_by_id(task_id).get_utilization())
    utilization_max = get_task_by_id(task_id).get_utilization();
  if (density_max < get_task_by_id(task_id).get_density())
    density_max = get_task_by_id(task_id).get_density();
}

void DAG_TaskSet::add_task(DAG_Task dag_task) {
  dag_tasks.push_back(dag_task);
  utilization_sum += dag_task.get_utilization();
  density_sum += dag_task.get_density();
  if (utilization_max < dag_task.get_utilization())
    utilization_max = dag_task.get_utilization();
  if (density_max < dag_task.get_density())
    density_max = dag_task.get_density();
}

DAG_TaskSet::~DAG_TaskSet() { dag_tasks.clear(); }

DAG_Tasks &DAG_TaskSet::get_tasks() { return dag_tasks; }

DAG_Task &DAG_TaskSet::get_task_by_id(uint id) {
  foreach(dag_tasks, task) {
    if (id == task->get_id()) return (*task);
  }
  return *reinterpret_cast<DAG_Task *>(0);
}

uint DAG_TaskSet::get_taskset_size() const { return dag_tasks.size(); }

fraction_t DAG_TaskSet::get_utilization_sum() const { return utilization_sum; }
fraction_t DAG_TaskSet::get_utilization_max() const { return utilization_max; }
fraction_t DAG_TaskSet::get_density_sum() const { return density_sum; }
fraction_t DAG_TaskSet::get_density_max() const { return density_max; }

void DAG_TaskSet::sort_by_period() {
  std::sort(dag_tasks.begin(), dag_tasks.end(), period_increase<DAG_Task>);
  // for (uint i = 0; i < dag_tasks.size(); i++)
  //  dag_tasks[i].set_id(i);
}

/** Others */

void task_gen(TaskSet *taskset, ResourceSet *resourceset, Param param,
              double utilization) {
  while (taskset->get_utilization_sum() < utilization) {  // generate tasks
    ulong period =
        Random_Gen::uniform_integral_gen(static_cast<int>(param.p_range.min),
                                         static_cast<int>(param.p_range.max));
    // fraction_t u = Random_Gen::uniform_real_gen(0.2, 0.5);
    fraction_t u = Random_Gen::exponential_gen(param.lambda);
    ulong wcet = period * u.get_d();
    if (0 == wcet)
      wcet++;
    else if (wcet > period)
      wcet = period;
    ulong deadline = 0;
    if (fabs(param.d_range.max) > _EPS) {
      deadline = ceil(period * Random_Gen::uniform_real_gen(param.d_range.min,
                                                            param.d_range.max));
      // if (deadline > period)
      //    deadline = period;
      if (deadline < wcet) deadline = wcet;
    }
    fraction_t temp(wcet, period);
    if (taskset->get_utilization_sum() + temp > utilization) {
      temp = utilization - taskset->get_utilization_sum();
      wcet = period * temp.get_d() + 1;
      if (deadline != 0 && deadline < wcet) deadline = wcet;
      // taskset->add_task(wcet, period);
      taskset->add_task(resourceset, param, wcet, period, deadline);
      break;
    }
    // taskset->add_task(wcet,period);
    taskset->add_task(resourceset, param, wcet, period, deadline);
  }
  taskset->sort_by_period();
}

void task_gen_UUnifast_Discard(TaskSet *taskset, ResourceSet *resourceset,
                               Param param, double utilization) {
  fraction_t SumU;

  // uint n = 10;
  uint n = param.lambda * utilization;

  vector<fraction_t> u_set;
  bool re_gen = true;

  while (re_gen) {
    SumU = utilization;
    re_gen = false;
    for (uint i = 1; i < n; i++) {
      double temp = Random_Gen::uniform_real_gen(0, 1);
      fraction_t NextSumU = SumU * pow(temp, 1.0 / (n - i));
      fraction_t u = SumU - NextSumU;
      if ((u - 1) >= _EPS) {
        u_set.clear();
        re_gen = true;
        cout << "Regenerate taskset." << endl;
        break;
      } else {
        u_set.push_back(u);
        SumU = NextSumU;
      }
    }
    if (!re_gen) {
      if ((SumU - 1) >= _EPS) {
          u_set.clear();
          re_gen = true;
          cout << "Regenerate taskset." << endl;
      } else {
          u_set.push_back(SumU);
      }
    }
  }

  uint i = 0;
  foreach(u_set, u) {
    // cout << "U_" << (i++) << ":" << (*u).get_d() << endl;

    ulong period =
        Random_Gen::uniform_integral_gen(static_cast<int>(param.p_range.min),
                                         static_cast<int>(param.p_range.max));
    ulong wcet = ceil((*u).get_d() * period);
    if (0 == wcet)
      wcet++;
    else if (wcet > period)
      wcet = period;
    ulong deadline = 0;
    if (fabs(param.d_range.max) > _EPS) {
      deadline = ceil(period * Random_Gen::uniform_real_gen(param.d_range.min,
                                                            param.d_range.max));
      // if (deadline > period)
      //    deadline = period;
      if (deadline < wcet) deadline = wcet;
    }

    taskset->add_task(resourceset, param, wcet, period, deadline);
  }

  // cout << "U_sum:" << taskset->get_utilization_sum().get_d() << endl;

  taskset->sort_by_period();
}

void task_load(TaskSet* taskset, ResourceSet* resourceset, string file_name) {
  ifstream file(file_name, ifstream::in);
  string buf;
  uint id = 0;
  while (getline(file, buf)) {
    // cout << buf <<endl;
    vector<uint64_t> elements;
    extract_element(elements, buf, 1, 3 * (1 + resourceset->get_resourceset_size()));
    if (3 <= elements.size()) {
      Task task(id, elements[0], elements[1], elements[2]);
      uint n = (elements.size() - 3) / 3;
      // cout << n <<endl;
      for (uint i = 0; i < n; i++) {
        uint64_t length = elements[4 + i * 3] * elements[5 + i * 3];
        // cout << length<<endl;
        task.add_request(elements[3 + i * 3], elements[4 + i * 3], elements[5 + i * 3], length);
        task.set_wcet_non_critical_sections(task.get_wcet_non_critical_sections() - length);
        // cout<<task.get_wcet_non_critical_sections()<<endl;
        task.set_wcet_critical_sections(task.get_wcet_critical_sections() + length);
        // cout<<task.get_wcet_critical_sections()<<endl;
        resourceset->add_task(elements[3 + i * 3], id);
        // cout<<"add to resourceset"<<endl;
      }
      taskset->add_task(task);

      id++;
    }
  }
  taskset->sort_by_period();

  foreach(taskset->get_tasks(), task) {
    cout << "Task" << task->get_id()
          << ": partition:" << task->get_partition()
          << ": priority:" << task->get_priority() << endl;
    cout << "ncs-wcet:" << task->get_wcet_non_critical_sections()
          << " cs-wcet:" << task->get_wcet_critical_sections()
          << " wcet:" << task->get_wcet()
          << " response time:" << task->get_response_time()
          << " deadline:" << task->get_deadline()
          << " period:" << task->get_period() << endl;
    foreach(task->get_requests(), request) {
      cout << "request" << request->get_resource_id() << ":"
            << " num:" << request->get_num_requests()
            << " length:" << request->get_max_length() << " locality:"
            << resourceset->get_resources()[request->get_resource_id()]
                  .get_locality()
            << endl;
    }
    cout << "-------------------------------------------" << endl;
    if (task->get_wcet() > task->get_response_time()) exit(0);
  }
}

void dag_task_gen(DAG_TaskSet *dag_taskset, ResourceSet *resourceset,
                  Param param, double utilization) {
  // generate tasks
  while (dag_taskset->get_utilization_sum() < utilization) {
    ulong period =
        Random_Gen::uniform_integral_gen(static_cast<int>(param.p_range.min),
                                         static_cast<int>(param.p_range.max));
    fraction_t u = Random_Gen::exponential_gen(param.lambda);
    ulong wcet = period * u.get_d();
    if (0 == wcet)
      wcet++;
    else if (wcet > period)
      wcet = period;
    ulong deadline = 0;
    if (fabs(param.d_range.max) > _EPS) {
      deadline = ceil(period * Random_Gen::uniform_real_gen(param.d_range.min,
                                                            param.d_range.max));
      // if (deadline > period)
      //     deadline = period;
      if (deadline < wcet) deadline = wcet;
    }
    fraction_t temp(wcet, period);
    if (dag_taskset->get_utilization_sum() + temp > utilization) {
      temp = utilization - dag_taskset->get_utilization_sum();
      wcet = period * temp.get_d() + 1;
      if (deadline != 0 && deadline < wcet) deadline = wcet;
      dag_taskset->add_task(resourceset, param, wcet, period, deadline);
      break;
    }
    dag_taskset->add_task(resourceset, param, wcet, period, deadline);
  }

  foreach(dag_taskset->get_tasks(), task) { task->refresh_relationship(); }

  // dag_taskset.sort_by_period();
}

ulong gcd(ulong a, ulong b) {
  ulong temp;
  while (b) {
    temp = a;
    a = b;
    b = temp % b;
  }
  return a;
}

ulong lcm(ulong a, ulong b) { return a * b / gcd(a, b); }

void extract_element(vector<uint64_t>& elements, string bufline, uint start, uint num, string seperator) {
  char *charbuf;
  string cut = " \t\r\n";
  cut += seperator;

  uint count = 0;
  try {
    if (NULL != (charbuf = strtok(bufline.data(), cut.data())))
      do {
        if (count >= start && count < start + num) {
          // cout<<" "<<charbuf<<endl;
          uint64_t element = atol(charbuf);
          // cout<<" "<<element<<endl;
          elements.push_back(element);
        }
        count++;
      } while (NULL != (charbuf = strtok(NULL, cut.data())));
  } catch(exception &e) {
    cout << "extract exception." << endl;
  }
}
