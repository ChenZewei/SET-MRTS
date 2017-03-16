#ifndef ILP_SPINLOCK_H
#define ILP_SPINLOCK_H

#include "types.h"
#include "varmapper.h"

class Task;
class TaskSet;
class Request;
class Resource;
class ResourceSet;
class ProcessorSet;
class LinearExpression;
class LinearProgram;

/*
|________________|_______________|_____________|_____________|_____________|____________|
|                |               |             |             |             |            |
|(63-42)Reserved |(43-40)var type|(39-30)part1 |(29-20)part2 |(19-10)part3 |(9-0)part4  |
|________________|_______________|_____________|_____________|_____________|____________|
*/

class ILPSpinLockMapper: public VarMapperBase
{
	public:
		enum var_type
		{
			LOCALITY_ASSIGNMENT,//A_i_x Task i assign on processor x(x start from 1)
			PRIORITY_ASSIGNMENT,//Pi_i_p Priority p assign to Task i(p start from 1)
			SAME_LOCALITY,//V_i_x Task i and task x assigned on same processor
			HIGHER_PRIORITY,//X_i_x Ti has higher priority than Tx
			MAX_PREEMEPT,//,//H_i_x 
			INTERFERENCE_TIME,//I_i
			SPIN_TIME,//S_i_k
			BLOCKING_TIME,//B_i_q_k
			AB_DEISION,//Z_i_q
			RESPONSE_TIME//R_i
		};
	private:
		static uint64_t encode_request(uint64_t type, uint64_t part_1 = 0, uint64_t part_2 = 0, uint64_t part_3 = 0, uint64_t part_4 = 0);
		static uint64_t get_type(uint64_t var);
		static uint64_t get_part_1(uint64_t var);
		static uint64_t get_part_2(uint64_t var);
		static uint64_t get_part_3(uint64_t var);
		static uint64_t get_part_4(uint64_t var);
	public:
		ILPSpinLockMapper(uint start_var = 0);
		uint lookup(uint type, uint part_1 = 0, uint part_2 = 0, uint part_3 = 0, uint part_4 = 0);
		string var2str(unsigned int var) const;
		string key2str(uint64_t key) const;
};

////////////////////SchedulabilityTest////////////////////
bool is_ILP_SpinLock_schedulable(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources);

void ILP_SpinLock_construct_exp(ILPSpinLockMapper& vars, LinearExpression *exp, uint type, uint part_1 = 0, uint part_2 = 0, uint part_3 = 0, uint part_4 = 0);

void ILP_SpinLock_set_objective(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

void ILP_SpinLock_add_constraints(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

////////////////////Expressions////////////////////
//C1 2013 SIES Alexander
void ILP_SpinLock_constraint_1(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

//C2 2013 SIES Alexander
void ILP_SpinLock_constraint_2(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

//C2-1 2013 SIES Alexander
void ILP_SpinLock_constraint_2_1(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);


//C3 2013 SIES Alexander
void ILP_SpinLock_constraint_3(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

//C4 2013 SIES Alexander
void ILP_SpinLock_constraint_4(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

//C5 2013 SIES Alexander
void ILP_SpinLock_constraint_5(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

//C6 2013 SIES Alexander
void ILP_SpinLock_constraint_6(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

//C7 2013 SIES Alexander
void ILP_SpinLock_constraint_7(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

/*
Lower bound of interference I_i
*/
//C8 2013 SIES Alexander
void ILP_SpinLock_constraint_8(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

//C9 2013 SIES Alexander
void ILP_SpinLock_constraint_9(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

//C10 2013 SIES Alexander
void ILP_SpinLock_constraint_10(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

/*
Lower bound of spin delay S_i
*/
//C11 2013 SIES Alexander
void ILP_SpinLock_constraint_11(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

//C12 2013 SIES Alexander
void ILP_SpinLock_constraint_12(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

//C13 2013 SIES Alexander
void ILP_SpinLock_constraint_13(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

/*
Lower bound of arrival blocking B_i
*/
//C14 2013 SIES Alexander
void ILP_SpinLock_constraint_14(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

//C15 2013 SIES Alexander
void ILP_SpinLock_constraint_15(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

//C16 2013 SIES Alexander
void ILP_SpinLock_constraint_16(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

//C17 2013 SIES Alexander
void ILP_SpinLock_constraint_17(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

//C18 2013 SIES Alexander
void ILP_SpinLock_constraint_18(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);

//C19 2013 SIES Alexander
void ILP_SpinLock_constraint_19(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, ILPSpinLockMapper& vars);







#endif
