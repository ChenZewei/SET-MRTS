#ifndef ILP_RTA_PFP_DPCP_H
#define ILP_RTA_PFP_DPCP_H

#include "types.h"
#include "varmapper.h"
#include "p_sched.h"
#include "tasks.h"
#include "processors.h"
#include "resources.h"

/*
|________________|_______________|_____________|_____________|_____________|____________|
|                |               |             |             |             |            |
|(63-42)Reserved |(43-40)var type|(39-30)part1 |(29-20)part2 |(19-10)part3 |(9-0)part4  |
|________________|_______________|_____________|_____________|_____________|____________|
*/

class ILPDPCPMapper: public VarMapperBase
{
	public:
		enum var_type
		{
			LOCALITY_ASSIGNMENT,//A_i_k Task i assign on processor k(k start from 1)
			RESOURCE_ASSIGNMENT,//Q_i_k resource i assign to processor x(k start from 1)
			SAME_TASK_LOCALITY,//U_i_x Task i and task x assigned on same processor
			SAME_RESOURCE_LOCALITY,//V_u_v resource u and resource v assigned on same processor
			SAME_TR_LOCALITY,//W_i_u task i and resource u assigned on same processor
			APPLICATION_CORE,//AP_k
			UNIFORM_CORE,//UC_k
			RESPONSE_TIME//R_i
			BLOCKING_TIME,//B_i
			REQUEST_BLOCKING_TIME,//b_i_r
			INTERFERENCE_TIME_R,//IR_i
			INTERFERENCE_TIME_R_PROCESSOR,//IR_i_k
			WORKLOAD_R,//WR_i_x_v
			INTERFERENCE_TIME_C,//IR_c
			WORKLOAD_C//WC_i_j
		};
	private:
		static uint64_t encode_request(uint64_t type, uint64_t part_1 = 0, uint64_t part_2 = 0, uint64_t part_3 = 0, uint64_t part_4 = 0);
		static uint64_t get_type(uint64_t var);
		static uint64_t get_part_1(uint64_t var);
		static uint64_t get_part_2(uint64_t var);
		static uint64_t get_part_3(uint64_t var);
		static uint64_t get_part_4(uint64_t var);
	public:
		ILPDPCPMapper(uint start_var = 0);
		uint lookup(uint type, uint part_1 = 0, uint part_2 = 0, uint part_3 = 0, uint part_4 = 0);
		string var2str(unsigned int var) const;
		string key2str(uint64_t key) const;
};

////////////////////ILP_PFP_DPCP////////////////////

class ILP_RTA_PFP_DPCP: public PartitionedSched
{
	private:
		TaskSet tasks;
		ProcessorSet processors;
		ResourceSet resources;

		void construct_exp(ILPDPCPMapper& vars, LinearExpression *exp, uint type, uint part_1 = 0, uint part_2 = 0, uint part_3 = 0, uint part_4 = 0);
		void set_objective(LinearProgram& lp, ILPDPCPMapper& vars);
		void add_constraints(LinearProgram& lp, ILPDPCPMapper& vars);

		//Constraints
		//C1
		void constraint_1(LinearProgram& lp, ILPDPCPMapper& vars);
		//C2
		void constraint_2(LinearProgram& lp, ILPDPCPMapper& vars);
		//C3
		void constraint_3(LinearProgram& lp, ILPDPCPMapper& vars);
		//C4
		void constraint_4(LinearProgram& lp, ILPDPCPMapper& vars);
		//C5
		void constraint_5(LinearProgram& lp, ILPDPCPMapper& vars);
		//C6
		void constraint_6(LinearProgram& lp, ILPDPCPMapper& vars);
		//C7
		void constraint_7(LinearProgram& lp, ILPDPCPMapper& vars);
		//C8
		void constraint_8(LinearProgram& lp, ILPDPCPMapper& vars);
		//C9
		void constraint_9(LinearProgram& lp, ILPDPCPMapper& vars);
		//C10
		void constraint_10(LinearProgram& lp, ILPDPCPMapper& vars);
		//C11
		void constraint_11(LinearProgram& lp, ILPDPCPMapper& vars);
		//C12
		void constraint_12(LinearProgram& lp, ILPDPCPMapper& vars);
		//C13
		void constraint_13(LinearProgram& lp, ILPDPCPMapper& vars);
		//C14
		void constraint_14(LinearProgram& lp, ILPDPCPMapper& vars);
		//C15
		void constraint_15(LinearProgram& lp, ILPDPCPMapper& vars);
		//C16
		void constraint_16(LinearProgram& lp, ILPDPCPMapper& vars);

		bool alloc_schedulable();

	public:
		ILP_RTA_PFP_DPCP();
		ILP_RTA_PFP_DPCP(TaskSet tasks, ProcessorSet processors, ResourceSet resources);
		~ILP_RTA_PFP_DPCP();
		bool is_schedulable();
};

#endif































