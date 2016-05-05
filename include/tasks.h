#ifndef TASKS_H
#define TASKS_H

#include <math.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include "types.h"
using namespace std;

typedef vector<fraction_t> Ratio;

class Task
{
	private:
		uint id;
		ulong wcet;
		ulong wcet_critical_sections;
		ulong wcet_non_critical_sections;
		ulong spin;
		ulong self_suspension;
		ulong local_blocking;
		ulong total_blocking;
		ulong jitter;
		ulong response_time;
		ulong deadline;
		ulong period;
		uint priority;
		uint partition;
		bool independent;
		fraction_t utilization;
		fraction_t density;
		Ratio ratio;//for heterogeneous platform
	public:
		Task(uint id,
			ulong wcet, 
			ulong period,
			ulong deadline = 0,
			uint priority = 0);
		
		uint get_id() const { return id; }
		ulong get_wcet() const	{ return wcet; }
		ulong get_wcet_critical_sections() const { return wcet_critical_sections; }
		ulong get_wcet_non_critical_sections() const {	return wcet_non_critical_sections; }
		ulong get_spin() const	{ return spin; }
		ulong get_local_blocking() const { return local_blocking; }
		ulong get_total_blocking() const { return total_blocking; }
		ulong get_self_suspension() const { return self_suspension; }
		ulong get_jitter() const { return jitter; }
		ulong get_response_time() const { return response_time; }
		ulong get_deadline() const { return deadline; }
		ulong get_period() const { return period; }
		uint get_priority() const { return priority; }
		uint get_partition() const { return partition; }
		bool is_independent() const { return independent; }
		bool is_feasible() const 
		{
			return deadline >= wcet && period >= wcet && wcet > 0;
		}
		
		ulong DBF(ulong interval);//Demand Bound Function
		uint get_max_num_jobs(ulong interval);//max number of jobs in an arbitrary length of interval
		void DBF();
		fraction_t get_utilization();
		fraction_t get_density();
		void get_utilization(fraction_t &utilization);
		void get_density(fraction_t &density);
	
};

class Request
{
private:
	uint resource_id;
	uint num_requests;
	ulong max_length;
	ulong total_length;
	const Task*    task;

public:
	Request(uint res_id,
		uint num,
		ulong max_len,
		ulong total_len,
		const Task* tsk);

	uint get_resource_id() const { return resource_id; }
	uint get_num_requests() const { return num_requests; }
	ulong get_max_length() const { return max_length; }
	ulong get_total_length() const { return total_length; }
	const Task* get_task() const { return task; }
	ulong get_max_num_requests(ulong interval) const;
};

typedef vector<uint> Jobs;//wcet

struct Edge//job m can be excuted only if job v is completed
{
	uint v;
	uint m;
};

typedef vector<struct Edge> Edges;

typedef struct
{
	Jobs jobs;
	Edges edges;
}Graph;


class DAG_Task:public Task
{
	private:
		Graph graph;
		uint vol;//total wcet of the jobs in graph
	public:
		void update_vol();
		bool is_acyclic();
		uint DBF(uint time);//Demand Bound Function
		void DBF();
		fraction_t get_utilization();
		fraction_t get_density();
		void get_utilization(fraction_t &utilization);
		void get_density(fraction_t &density);	
};

typedef vector<Task> Tasks;

#define foreach(tasks, condition) 		\
	for(int i; i < tasks.size(); i++)	\
	{					\
		if(condition)			\
			return false;		\
	}

class TaskSet
{
	private:
		Tasks tasks;
		
		fraction_t utilization_sum;
		fraction_t utilization_max;
		fraction_t density_sum;
		fraction_t density_max;
	public:
		TaskSet();
		~TaskSet()
		{
			tasks.clear();
		}
		void add_task(uint wcet, uint period, uint deadline = 0)
		{
			fraction_t utilization_new = wcet, density_new = wcet;
			utilization_new /= period;
			if(0 == deadline)
				density_new /= period;
			else
				density_new /= min(deadline, period);
			tasks.push_back(Task(tasks.size(), wcet, period, deadline));
			utilization_sum += utilization_new;
			density_sum += density_new;
			if(utilization_max < utilization_new)
				utilization_max = utilization_new;
			if(density_max < density_new)
				density_max = density_new;
		}
		bool is_implicit_deadline()
		{
			foreach(tasks,tasks[i].get_deadline() != tasks[i].get_period());
			return true;
		}
		bool is_constraint_deadline()
		{
			foreach(tasks,tasks[i].get_deadline() > tasks[i].get_period());
			return true;
		}
		bool is_arbitary_deadline()
		{
			return !(is_implicit_deadline())&&!(is_constraint_deadline());
		}
		uint get_taskset_size()
		{
			return tasks.size();
		}

		fraction_t get_task_utilization(uint index)
		{
			return tasks[index].get_utilization();
		}
		fraction_t get_task_density(uint index)
		{
			return tasks[index].get_density();
		}
		uint get_task_wcet(uint index)
		{
			return tasks[index].get_wcet();
		}
		uint get_task_deadline(uint index)
		{
			return tasks[index].get_deadline();
		}
		uint get_task_period(uint index)
		{
			return tasks[index].get_period();
		}
		
		fraction_t get_utilization_sum();
		fraction_t get_utilization_max();
		fraction_t get_density_sum();
		fraction_t get_density_max();
		void get_utilization_sum(fraction_t &utilization_sum);
		void get_utilization_max(fraction_t &utilization_max);
		void get_density_sum(fraction_t &density_sum);
		void get_density_max(fraction_t &density_max);

		uint DBF(uint time);
};



#endif
