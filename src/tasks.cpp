#include "tasks.h"
#include <vector>

#include <iostream>
////////////////////////////Task//////////////////////////////

uint Task::DBF(uint time)
{
	if(time >= deadline)
		return ((time - deadline)/period+1)*wcet;
	else 
		return 0;
}


 
fraction_t Task::get_utilization()
{
	return utilization;
}

fraction_t Task::get_density()
{
	return density;
}

void Task::get_utilization(fraction_t &utilization)
{
	*utilization = wcet;
	*utilization /= period;
}

void Task::get_density(fraction_t &density)
{
	*density = wcet;
	*density /= min(deadline,period);
}

/////////////////////////////TaskSet///////////////////////////////

TaskSet()
{
	utilization_sum = 0;
	utilization_max = 0;
	density_sum = 0;
	density_max = 0;
}

void TaskSet::get_utilization_sum(fraction_t &utilization_sum)
{
	fraction_t temp;
	*utilization_sum = 0;
	for(int i; i < tasks.size(); i++)
	{
		temp = tasks[i].wcet;
		temp /= tasks[i].get_period();
		*utilization_sum += temp;
	}
}
void TaskSet::get_utilization_max(fraction_t &utilization_max)
{
	*utilization_max = tasks[0].get_utilization();
	for(int i = 1; i < tasks.size(); i++)
		if(tasks[i].get_utilization() > *utilization_max)
			*utilization_max = tasks[i].get_utilization();
}
bool TaskSet::get_density_sum(fraction_t &density_sum)
{
	fraction_t temp;
	*get_density_sum = 0;
	for(int i; i < tasks.size(); i++)
	{
		temp = tasks[i].wcet;
		temp /= min(tasks[i].get_deadline(),tasks[i].get_period());
		*get_density_sum += temp;
	}
}
void get_density_max(fraction_t &density_max)
{
	*density_max = tasks[0].get_density();
	for(int i = 1; i < tasks.size(); i++)
		if(tasks[i].get_density() > *density_max)
			*density_max = tasks[i].get_density();
}
uint TaskSet::DBF(uint time)
{
	
}