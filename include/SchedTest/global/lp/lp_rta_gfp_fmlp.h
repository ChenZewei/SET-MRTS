#ifndef LP_RTA_GFP_FMLP_H
#define LP_RTA_GFP_FMLP_H

/*
** LinearProgramming approach for global fix-priority scheduling under FMLP locking protocol
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
class FMLPMapper: public VarMapperBase
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
		FMLPMapper(uint start_var = 0);
		uint lookup(uint task_id, uint res_id, uint req_id, var_type type);
		string key2str(uint64_t key, uint var) const;
};


class LP_RTA_GFP_FMLP: public GlobalSched
{
	private:
		TaskSet tasks;
		ProcessorSet processors;
		ResourceSet resources;
		
		ulong fmlp_get_response_time(Task& ti);
		ulong fmlp_workload_bound(Task& tx, ulong Ri);
//		ulong fmlp_holding_bound(Task& ti, Task& tx, uint resource_id);
//		ulong fmlp_wait_time_bound(Task& ti, uint resource_id)
		void lp_fmlp_directed_blocking(Task& ti, Task& tx, FMLPMapper& vars, LinearExpression *exp, double coef = 1);
		void lp_fmlp_indirected_blocking(Task& ti, Task& tx, FMLPMapper& vars, LinearExpression *exp, double coef = 1);
		void lp_fmlp_preemption_blocking(Task& ti, Task& tx, FMLPMapper& vars, LinearExpression *exp, double coef = 1);
		void lp_fmlp_OD(Task& ti, FMLPMapper& vars, LinearExpression *exp, double coef = 1);
		void lp_fmlp_declare_variable_bounds(Task& ti, LinearProgram& lp, FMLPMapper& vars);
		void lp_fmlp_objective(Task& ti, LinearProgram& lp, FMLPMapper& vars, LinearExpression *obj);
		void lp_fmlp_add_constraints(Task& ti, LinearProgram& lp, FMLPMapper& vars);
		//Constraint 1 [Maolin 2015 RTSS]
		void lp_fmlp_constraint_1(Task& ti, LinearProgram& lp, FMLPMapper& vars);
		//Constraint 2 [Maolin 2015 RTSS]
		void lp_fmlp_constraint_2(Task& ti, LinearProgram& lp, FMLPMapper& vars);
		//Constraint 3 [Maolin 2015 RTSS]
		void lp_fmlp_constraint_3(Task& ti, LinearProgram& lp, FMLPMapper& vars);
		//Constraint 4 [Maolin 2015 RTSS]
		void lp_fmlp_constraint_4(Task& ti, LinearProgram& lp, FMLPMapper& vars);
		//Constraint 5 [Maolin 2015 RTSS]
		void lp_fmlp_constraint_5(Task& ti, LinearProgram& lp, FMLPMapper& vars);
		//Constraint 6 [Maolin 2015 RTSS]
		void lp_fmlp_constraint_6(Task& ti, LinearProgram& lp, FMLPMapper& vars);
		//Constraint 7 [Maolin 2015 RTSS]
		void lp_fmlp_constraint_7(Task& ti, LinearProgram& lp, FMLPMapper& vars);
		//Constraint 8 [Maolin 2015 RTSS]
		void lp_fmlp_constraint_8(Task& ti, LinearProgram& lp, FMLPMapper& vars);
		//Constraint 11 [Maolin 2015 RTSS]
		void lp_fmlp_constraint_9(Task& ti, LinearProgram& lp, FMLPMapper& vars);
		//Constraint 13 [Maolin 2015 RTSS]
		void lp_fmlp_constraint_10(Task& ti, LinearProgram& lp, FMLPMapper& vars);
	public:
		LP_RTA_GFP_FMLP();
		LP_RTA_GFP_FMLP(TaskSet tasks, ProcessorSet processors, ResourceSet resources);
		~LP_RTA_GFP_FMLP();
		bool is_schedulable();
};

#endif
