#ifndef LP_RTA_GFP_PIP_H
#define LP_RTA_GFP_PIP_H

/*
** LinearProgramming approach for global fix-priority scheduling under PIP locking protocol
** 
** RTSS 2015 Maolin Yang et al. [Global Real-Time Semaphore Protocols: A Survey, Unified Analysis, and Comparison]
*/

#include "g_sched.h"
#include "varmapper.h"
#include "tasks.h"
#include "processors.h"
#include "resources.h"

class Task;
class TaskSet;
class Request;
class Resource;
class ResourceSet;
class ProcessorSet;
class LinearExpression;
class LinearProgram;

/*
|________________|_____________________|______________________|______________________|______________________|
|                |                     |                      |                      |                      |
|(63-34)Reserved |(32-30) var type     |(29-20) Task          |(19-10) Resource      |(9-0) Request         |
|________________|_____________________|______________________|______________________|______________________|
*/
class PIPMapper: public VarMapperBase
{
	public:
		enum var_type
		{
			BLOCKING_DIRECT,//0x000
			BLOCKING_INDIRECT,//0x001
			BLOCKING_PREEMPT,//0x010
			BLOCKING_OTHER,//0x011
			INTERF_REGULAR,//0x100
			INTERF_CO_BOOSTING,//0x101
			INTERF_STALLING,//0x110
			INTERF_OTHER//0x111
		};
	
	private:	
		static uint64_t encode_request(uint64_t task_id, uint64_t res_id, uint64_t req_id, uint64_t type);
		static uint64_t get_type(uint64_t var);
		static uint64_t get_task(uint64_t var);
		static uint64_t get_res_id(uint64_t var);
		static uint64_t get_req_id(uint64_t var);
	public:
		PIPMapper(uint start_var = 0);
		uint lookup(uint task_id, uint res_id, uint req_id, var_type type);
		string key2str(uint64_t key, uint var) const;
};


class LP_RTA_GFP_PIP: public GlobalSched
{
	private:
		TaskSet tasks;
		ProcessorSet processors;
		ResourceSet resources;
		
		ulong response_time(Task& ti);
		ulong workload_bound(Task& tx, ulong Ri);
		ulong holding_bound(Task& ti, Task& tx, uint resource_id);
		ulong wait_time_bound(Task& ti, uint resource_id);
		void lp_pip_directed_blocking(Task& ti, Task& tx, PIPMapper& vars, LinearExpression *exp, double coef = 1);
		void lp_pip_indirected_blocking(Task& ti, Task& tx, PIPMapper& vars, LinearExpression *exp, double coef = 1);
		void lp_pip_preemption_blocking(Task& ti, Task& tx, PIPMapper& vars, LinearExpression *exp, double coef = 1);
		void lp_pip_OD(Task& ti, PIPMapper& vars, LinearExpression *exp, double coef = 1);
		void lp_pip_declare_variable_bounds(Task& ti, LinearProgram& lp, PIPMapper& vars);
		void lp_pip_objective(Task& ti, LinearProgram& lp, PIPMapper& vars, LinearExpression *obj);
		void lp_pip_add_constraints(Task& ti, LinearProgram& lp, PIPMapper& vars);
		//Constraint 1 [Maolin 2015 RTSS]
		void lp_pip_constraint_1(Task& ti, LinearProgram& lp, PIPMapper& vars);
		//Constraint 2 [Maolin 2015 RTSS]
		void lp_pip_constraint_2(Task& ti, LinearProgram& lp, PIPMapper& vars);
		//Constraint 3 [Maolin 2015 RTSS]
		void lp_pip_constraint_3(Task& ti, LinearProgram& lp, PIPMapper& vars);
		//Constraint 4 [Maolin 2015 RTSS]
		void lp_pip_constraint_4(Task& ti, LinearProgram& lp, PIPMapper& vars);
		//Constraint 5 [Maolin 2015 RTSS]
		void lp_pip_constraint_5(Task& ti, LinearProgram& lp, PIPMapper& vars);
		//Constraint 6 [Maolin 2015 RTSS]
		void lp_pip_constraint_6(Task& ti, LinearProgram& lp, PIPMapper& vars);
		//Constraint 7 [Maolin 2015 RTSS]
		void lp_pip_constraint_7(Task& ti, LinearProgram& lp, PIPMapper& vars);
		//Constraint 9 [Maolin 2015 RTSS]
		void lp_pip_constraint_8(Task& ti, LinearProgram& lp, PIPMapper& vars);
		//Constraint 10 [Maolin 2015 RTSS]
		void lp_pip_constraint_9(Task& ti, LinearProgram& lp, PIPMapper& vars);
		//Constraint 11 [Maolin 2015 RTSS]
		void lp_pip_constraint_10(Task& ti, LinearProgram& lp, PIPMapper& vars);
		//Constraint 12 [Maolin 2015 RTSS]
		void lp_pip_constraint_11(Task& ti, LinearProgram& lp, PIPMapper& vars);
	public:
		LP_RTA_GFP_PIP();
		LP_RTA_GFP_PIP(TaskSet tasks, ProcessorSet processors, ResourceSet resources);
		~LP_RTA_GFP_PIP();
		bool is_schedulable();
	
};

#endif
