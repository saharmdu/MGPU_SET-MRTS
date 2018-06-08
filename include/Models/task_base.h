#ifndef TASK_BASE_H
#define TASK_BASE_H

#include "types.h"

class ResourceSet;
class Param;

class TaskBase
{
	private:
		ulong wcet;
		ulong deadline;
		ulong period;
	public:
		TaskBase();
		TaseBase(ulong wcet, ulong period, ulong deadline = 0);
};

class TaskSetBase
{
	private:
		fraction_t utilization_sum;
		fraction_t utilization_max;
		fraction_t density_sum;
		fraction_t density_max;
	public:
		TaskSetBase();

		virtual uint get_size() const = 0;
		virtual void add_task(ResourceSet& resourceset, Param& param, long wcet, long period, long deadline = 0) = 0;
		virtual fraction_t get_utilization_sum() const = 0;
		virtual fraction_t get_utilization_max() const = 0;
		virtual fraction_t get_density_sum() const = 0;
		virtual fraction_t get_density_max() const = 0;
};

#endif
