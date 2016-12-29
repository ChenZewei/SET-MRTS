#include "lp_pip.h"
#include <tasks.h>
#include <resources.h>
#include <processors.h>
#include <lp.h>
#include <solution.h>
#include <sstream>
#include <iostream>
#include <assert.h>

////////////////////PIPMapper////////////////////
uint64_t PIPMapper::encode_request(uint64_t task_id, uint64_t res_id, uint64_t req_id, uint64_t type)
{
	uint64_t one = 1;
	uint64_t key = 0;
	assert(task_id < (one << 10));
	assert(res_id < (one << 10));
	assert(req_id < (one << 10));
	assert(type < (one << 3));

	key |= (type << 30);
	key |= (task_id << 20);
	key |= (res_id << 10);
	key |= req_id;
	return key;
}

uint64_t PIPMapper::get_type(uint64_t var)
{
	return (var >> 30) & (uint64_t) 0x7; //3 bits
}

uint64_t PIPMapper::get_task(uint64_t var)
{
	return (var >> 20) & (uint64_t) 0x3ff; //10 bits
}

uint64_t PIPMapper::get_res_id(uint64_t var)
{
	return (var >> 10) & (uint64_t) 0x3ff; //10 bits
}

uint64_t PIPMapper::get_req_id(uint64_t var)
{
	return var & (uint64_t) 0x3ff; //10 bits
}

PIPMapper::PIPMapper(uint start_var): VarMapperBase(start_var) {}

uint PIPMapper::lookup(uint task_id, uint res_id, uint req_id, var_type type)
{
	uint64_t key = encode_request(task_id, res_id, req_id, type);
	uint var = var_for_key(key);
	return var;
}

string PIPMapper::key2str(uint64_t key, uint var) const
{
	ostringstream buf;

	switch (get_type(key))
	{
		case PIPMapper::BLOCKING_DIRECT:
			buf << "Xd[";
			break;
		case PIPMapper::BLOCKING_INDIRECT:
			buf << "Xi[";
			break;
		case PIPMapper::BLOCKING_PREEMPT:
			buf << "Xp[";
			break;
		case PIPMapper::BLOCKING_OTHER:
			buf << "Xo[";
			break;
		case PIPMapper::INTERF_REGULAR:
			buf << "Ir[";
			break;
		case PIPMapper::INTERF_CO_BOOSTING:
			buf << "Ic[";
			break;
		case PIPMapper::INTERF_STALLING:
			buf << "Is[";
			break;
		case PIPMapper::INTERF_OTHER:
			buf << "Io[";
			break;
		default:
			buf << "?[";
	}

	buf << get_task(key) << ", "
		<< get_res_id(key) << ", "
		<< get_req_id(key) << "]";

	return buf.str();
}

////////////////////SchedulabilityTest////////////////////
bool is_global_pip_schedulable(TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources)
{
	bool update;
	do
	{
		update = false;
		foreach(tasks.get_tasks(), ti)
		{
			
			ulong response_time = ti->get_response_time();
//cout<<"original response time:"<<response_time<<endl;
			ulong temp = get_response_time(*ti, tasks, processors, resources);


//cout<<"current response time:"<<temp<<endl;

			assert(temp >= response_time);
			if(temp > response_time)
			{
//cout<<"last response time:"<<response_time<<" current response time:"<<temp<<endl;
				response_time = temp;
				update = true;
			}

			if(response_time < ti->get_deadline())
			{
				ti->set_response_time(response_time);
			}
			else
				return false;
		}
	}
	while(update);
	return true;
}

ulong get_response_time(Task& ti, TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources)
{
	ulong reponse_time = 0;

	PIPMapper vars;
	LinearProgram response_bound;
	LinearExpression *obj = new LinearExpression();

	lp_pip_objective(ti, tasks, processors, response_bound, vars, obj);

	response_bound.set_objective(obj);

	lp_pip_add_constraints(ti, tasks, processors, resources, response_bound, vars);

	vars.seal();

	GLPKSolution *rb_solution = new GLPKSolution(response_bound, vars.get_num_vars());

	assert(rb_solution != NULL);

	if(rb_solution->is_solved())
	{
//cout<<"solved."<<endl;
		reponse_time = ti.get_wcet() + lrint(rb_solution->evaluate(*(response_bound.get_objective())));
	}
	else
cout<<"unsolved."<<endl;

#if GLPK_MEM_USAGE_CHECK == 1
	int peak;
	glp_mem_usage(NULL, &peak, NULL, NULL);
	cout<<"Peak memory usage:"<<peak<<endl; 
#endif

	delete rb_solution;
	return reponse_time;
}

////////////////////Expressions////////////////////
void lp_pip_directed_blocking(Task& ti, Task& tx, TaskSet& tasks, PIPMapper& vars, LinearExpression *exp, double coef)
{
	uint x = tx.get_id();
	foreach(tx.get_requests(), request)
	{
		uint q = request->get_resource_id();
		ulong length = request->get_max_length();
		foreach_request_instance(ti, tx, q, v)
		{
			uint var_id;

			var_id = vars.lookup(x, q, v, PIPMapper::BLOCKING_DIRECT);
			exp->add_term(var_id, coef*length);
		}
	}
}

void lp_pip_indirected_blocking(Task& ti, Task& tx, TaskSet& tasks, PIPMapper& vars, LinearExpression *exp, double coef)
{
	uint x = tx.get_id();
	foreach(tx.get_requests(), request)
	{
		uint q = request->get_resource_id();
		ulong length = request->get_max_length();
		foreach_request_instance(ti, tx, q, v)
		{
			uint var_id;

			var_id = vars.lookup(x, q, v, PIPMapper::BLOCKING_INDIRECT);
			exp->add_term(var_id, coef*length);
		}
	}
}

void lp_pip_preemption_blocking(Task& ti, Task& tx, TaskSet& tasks, PIPMapper& vars, LinearExpression *exp, double coef)
{
	uint x = tx.get_id();
	foreach(tx.get_requests(), request)
	{
		uint q = request->get_resource_id();
		ulong length = request->get_max_length();
		foreach_request_instance(ti, tx, q, v)
		{
			uint var_id;

			var_id = vars.lookup(x, q, v, PIPMapper::BLOCKING_PREEMPT);
			exp->add_term(var_id, coef*length);
		}
	}
}


void lp_pip_OD(Task& ti, TaskSet& tasks, ProcessorSet& processors, PIPMapper& vars, LinearExpression *exp, double coef)
{
	uint p_num = processors.get_processor_num();
	uint var_id;

	foreach_higher_priority_task(tasks.get_tasks(), ti, th)
	{
		uint h = th->get_id();
		
		var_id = vars.lookup(h, 0, 0, PIPMapper::INTERF_REGULAR);
		exp->add_term(var_id, coef*(1/p_num));
	}

	foreach_lower_priority_task(tasks.get_tasks(), ti, tl)
	{
		uint l = tl->get_id();
		
		var_id = vars.lookup(l, 0, 0, PIPMapper::INTERF_CO_BOOSTING);
		exp->add_term(var_id, coef*(1/p_num));
		
		var_id = vars.lookup(l, 0, 0, PIPMapper::INTERF_STALLING);
		exp->add_term(var_id, coef*(1/p_num));

		lp_pip_indirected_blocking(ti, *tl, tasks, vars, exp, coef*(1/p_num));

		lp_pip_preemption_blocking(ti, *tl, tasks, vars, exp, coef*(1/p_num));
	}
}
/*
void lp_pip_objective(Task& ti, TaskSet& tasks, ProcessorSet& processors, LinearProgram& lp, PIPMapper& vars, LinearExpression *obj)
{
	uint p_num = processors.get_processor_num();
	uint var_id;

	foreach_higher_priority_task(tasks.get_tasks(), ti, th)
	{
		uint h = th->get_id();
		
		var_id = vars.lookup(h, 0, 0, PIPMapper::INTERF_REGULAR);
		obj->add_term(var_id, 1/p_num);
	}

	foreach_lower_priority_task(tasks.get_tasks(), ti, tl)
	{
		uint l = tl->get_id();
		
		var_id = vars.lookup(l, 0, 0, PIPMapper::INTERF_CO_BOOSTING);
		obj->add_term(var_id, 1/p_num);
		
		var_id = vars.lookup(l, 0, 0, PIPMapper::INTERF_STALLING);
		obj->add_term(var_id, 1/p_num);

		lp_pip_indirected_blocking(ti, *tl, tasks, vars, obj, 1/p_num);

		lp_pip_preemption_blocking(ti, *tl, tasks, vars, obj, 1/p_num);
	}

	foreach_task_except(tasks.get_tasks(), ti, tx)
	{
		lp_pip_directed_blocking(ti, *tx, tasks, vars, obj, 1/p_num);
	}
	
	vars.seal();
}
*/

/*
void lp_pip_objective(Task& ti, TaskSet& tasks, ProcessorSet& processors, LinearProgram& lp, PIPMapper& vars, LinearExpression *obj)
{
	uint p_num = processors.get_processor_num();
	uint var_id;

	foreach_task_except(tasks.get_tasks(), ti, tx)
	{
		uint x = tx->get_id();
		
		var_id = vars.lookup(x, 0, 0, PIPMapper::INTERF_REGULAR);
		obj->add_term(var_id, 1/p_num);
		lp.declare_variable_bounds(var_id, true, 0, false, -1);

		var_id = vars.lookup(x, 0, 0, PIPMapper::INTERF_CO_BOOSTING);
		obj->add_term(var_id, 1/p_num);
		lp.declare_variable_bounds(var_id, true, 0, false, -1);
		
		var_id = vars.lookup(x, 0, 0, PIPMapper::INTERF_STALLING);
		obj->add_term(var_id, 1/p_num);
		lp.declare_variable_bounds(var_id, true, 0, false, -1);
		
		lp_pip_directed_blocking(ti, *tx, tasks, vars, obj, 1);

		lp_pip_indirected_blocking(ti, *tx, tasks, vars, obj, 1/p_num);

		lp_pip_preemption_blocking(ti, *tx, tasks, vars, obj, 1/p_num);
	}

	vars.seal();
}
*/

void lp_pip_objective(Task& ti, TaskSet& tasks, ProcessorSet& processors, LinearProgram& lp, PIPMapper& vars, LinearExpression *obj)
{
	uint p_num = processors.get_processor_num();
	uint var_id;

	foreach_higher_priority_task(tasks.get_tasks(), ti, th)
	{
		uint h = th->get_id();
		
		var_id = vars.lookup(h, 0, 0, PIPMapper::INTERF_REGULAR);
		obj->add_term(var_id, 1/p_num);
	}

	foreach_lower_priority_task(tasks.get_tasks(), ti, tl)
	{
		uint l = tl->get_id();
		
		var_id = vars.lookup(l, 0, 0, PIPMapper::INTERF_CO_BOOSTING);
		obj->add_term(var_id, 1/p_num);
		
		var_id = vars.lookup(l, 0, 0, PIPMapper::INTERF_STALLING);
		obj->add_term(var_id, 1/p_num);

		lp_pip_indirected_blocking(ti, *tl, tasks, vars, obj, 1/p_num);

		lp_pip_preemption_blocking(ti, *tl, tasks, vars, obj, 1/p_num);
	}

	foreach_task_except(tasks.get_tasks(), ti, tx)
	{
		lp_pip_directed_blocking(ti, *tx, tasks, vars, obj, 1);
	}

//	vars.seal();
}

ulong workload_bound(Task& tx, ulong Ri)
{
	ulong e = tx.get_wcet();
	ulong d = tx.get_deadline();
	ulong p = tx.get_period();
	ulong r = tx.get_response_time();
	assert(d >= r);

	ulong N = (Ri - e + r)/p;

	return N*e + min(e, Ri + r - e - N*p);
}

ulong holding_bound(Task& ti, Task& tx, uint resource_id, TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources)
{

	uint x = tx.get_id(), q = resource_id;
	uint p_num = processors.get_processor_num();
	ulong L_x_q = tx.get_request_by_id(q).get_max_length();
	ulong holding_time = L_x_q;

	if(p_num < tx.get_id())
	{
		uint y = min(ti.get_id(), tx.get_id());
		uint z = max(ti.get_id(), tx.get_id());	
		bool update;
		do
		{
			update = false;
			ulong temp = 0;

			foreach_higher_priority_task_then(tasks.get_tasks(), y, th)
			{
				temp += workload_bound(*th, holding_time);
			}

			foreach_lower_priority_task_then(tasks.get_tasks(), y, tl)
			{
				uint l = tl->get_id();
				if(z != l)
				{
					foreach(tl->get_requests(), request)
					{
						uint u = request->get_resource_id();
						Resource& resource = resources.get_resources()[u];
						if(y > resource.get_ceiling())
						{
							uint N_l_u = request->get_num_requests();
							uint L_l_u = request->get_max_length();
							temp += tl->get_max_job_num(holding_time)*N_l_u*L_l_u;
						}
					}
				}
			}	
			temp /= p_num;
			temp += L_x_q;
			assert(temp >= holding_time);
			if(temp > ti.get_deadline())
				return MAX_LONG;

			if(temp > holding_time)
			{
				update = true;
				holding_time = temp;
			}
		}
		while(update);
	}

	return holding_time;
}

ulong wait_time_bound(Task& ti, uint resource_id, TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources)
{
	ulong wait_time = 0;
	ulong holding_time_l = 0;

	foreach_lower_priority_task(tasks.get_tasks(), ti, tl)
	{

		ulong temp = 0;

		if(tl->is_request_exist(resource_id))
			temp = holding_bound(ti, *tl, resource_id, tasks, processors, resources);

		if(temp > holding_time_l)
			holding_time_l = temp;
	}

	if(holding_time_l == MAX_LONG)
	{
//cout<<"holding_time_l:"<<holding_time_l<<endl;
		return MAX_LONG;
	}

	bool update;
	do
	{
		update = false;
		ulong temp = 1 + holding_time_l;

		foreach_higher_priority_task(tasks.get_tasks(), ti, th)
		{
			if(th->is_request_exist(resource_id))
			{
				uint N_h_q = th->get_request_by_id(resource_id).get_num_requests();
				ulong H_h_q = holding_bound(ti, *th, resource_id, tasks, processors, resources);
				temp += th->get_max_job_num(wait_time)*N_h_q *H_h_q;
			}
		}

		if(temp > ti.get_deadline())
		{
//cout<<"temp > ti.get_deadline():"<<temp<<" > "<<ti.get_deadline()<<endl;
			return MAX_LONG;
		}

//cout<<"temp:"<<temp<<endl;
//cout<<"wait_time:"<<wait_time<<endl;
		assert(temp >= wait_time);

		if(temp != wait_time)
		{
			update = true;
			wait_time = temp;
		}
	}
	while(update);

	return wait_time;
}

void lp_pip_add_constraints(Task& ti, TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, PIPMapper& vars)
{
	lp_pip_constraint_1(ti, tasks, resources, lp, vars);
	lp_pip_constraint_2(ti, tasks, processors, resources, lp, vars);
	lp_pip_constraint_3(ti, tasks, resources, lp, vars);
	lp_pip_constraint_4(ti, tasks, resources, lp, vars);
	lp_pip_constraint_5(ti, tasks, resources, lp, vars);
	lp_pip_constraint_6(ti, tasks, resources, lp, vars);
	lp_pip_constraint_7(ti, tasks, processors, resources, lp, vars);
	lp_pip_constraint_8(ti, tasks, resources, lp, vars);
	lp_pip_constraint_9(ti, tasks, processors, resources, lp, vars);
	lp_pip_constraint_10(ti, tasks, resources, lp, vars);
	lp_pip_constraint_11(ti, tasks, resources, lp, vars);
}

void lp_pip_constraint_1(Task& ti, TaskSet& tasks, ResourceSet& resources, LinearProgram& lp, PIPMapper& vars)
{
	foreach_task_except(tasks.get_tasks(), ti, tx)
	{
		LinearExpression *exp = new LinearExpression();

		uint var_id;
		uint x = tx->get_id();
				
		var_id = vars.lookup(x, 0, 0, PIPMapper::INTERF_REGULAR);
		exp->add_var(var_id);

		var_id = vars.lookup(x, 0, 0, PIPMapper::INTERF_CO_BOOSTING);
		exp->add_var(var_id);

		var_id = vars.lookup(x, 0, 0, PIPMapper::INTERF_STALLING);
		exp->add_var(var_id);
		
		lp_pip_directed_blocking(ti, *tx, tasks, vars, exp);

		lp_pip_indirected_blocking(ti, *tx, tasks, vars, exp);

		lp_pip_preemption_blocking(ti, *tx, tasks, vars, exp);

		lp.add_inequality(exp, workload_bound(*tx, ti.get_response_time()));
	}
}

void lp_pip_constraint_2(Task& ti, TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, PIPMapper& vars)
{	
	foreach_task_except(tasks.get_tasks(), ti, tx)
	{
		LinearExpression *exp = new LinearExpression();
		uint var_id;
		uint x = tx->get_id();

		var_id = vars.lookup(x, 0, 0, PIPMapper::INTERF_REGULAR);
		exp->add_var(var_id);

		var_id = vars.lookup(x, 0, 0, PIPMapper::INTERF_CO_BOOSTING);
		exp->add_var(var_id);

		var_id = vars.lookup(x, 0, 0, PIPMapper::INTERF_STALLING);
		exp->add_var(var_id);

		lp_pip_indirected_blocking(ti, *tx, tasks, vars, exp);

		lp_pip_preemption_blocking(ti, *tx, tasks, vars, exp);

		lp_pip_OD(ti, tasks, processors, vars, exp, -1);

		lp.add_inequality(exp, 0);
	}
}

void lp_pip_constraint_3(Task& ti, TaskSet& tasks, ResourceSet& resources, LinearProgram& lp, PIPMapper& vars)
{
	foreach_task_except(tasks.get_tasks(), ti, tx)
	{
		uint x = tx->get_id();
		foreach(tx->get_requests(), request)
		{
			uint q = request->get_resource_id();
			foreach_request_instance(ti, *tx, q, v)
			{
				LinearExpression *exp = new LinearExpression();
				uint var_id;
				
				var_id = vars.lookup(x, q, v, PIPMapper::BLOCKING_DIRECT);
				exp->add_var(var_id);

				var_id = vars.lookup(x, q, v, PIPMapper::BLOCKING_INDIRECT);
				exp->add_var(var_id);

				var_id = vars.lookup(x, q, v, PIPMapper::BLOCKING_PREEMPT);
				exp->add_var(var_id);

				lp.add_inequality(exp, 1);
			}
		}
	}
}

void lp_pip_constraint_4(Task& ti, TaskSet& tasks, ResourceSet& resources, LinearProgram& lp, PIPMapper& vars)
{
	if(0 == ti.get_requests().size())
	{
		foreach_lower_priority_task(tasks.get_tasks(), ti, tx)
		{	
			LinearExpression *exp = new LinearExpression();
			uint x = tx->get_id();
			uint var_id;
		
			var_id = vars.lookup(x, 0, 0, PIPMapper::INTERF_STALLING);
			exp->add_var(var_id);

			lp.add_equality(exp, 0);
		}
	}
}

void lp_pip_constraint_5(Task& ti, TaskSet& tasks, ResourceSet& resources, LinearProgram& lp, PIPMapper& vars)
{
	LinearExpression *exp = new LinearExpression();
	
	foreach_task_except(tasks.get_tasks(), ti, tx)
	{
		uint x = tx->get_id();
		foreach(tx->get_requests(), request)
		{
			uint q = request->get_resource_id();
			if(!ti.is_request_exist(q))
			{
				foreach_request_instance(ti, *tx, q, v)
				{
					uint var_id;
				
					var_id = vars.lookup(x, q, v, PIPMapper::BLOCKING_DIRECT);
					exp->add_var(var_id);
				}
			}
		}
	}
	
	lp.add_equality(exp, 0);
}

void lp_pip_constraint_6(Task& ti, TaskSet& tasks, ResourceSet& resources, LinearProgram& lp, PIPMapper& vars)
{
	LinearExpression *exp = new LinearExpression();

	foreach_lower_priority_task(tasks.get_tasks(), ti, tx)
	{
		uint var_id;
		uint x = tx->get_id();

		var_id = vars.lookup(x, 0, 0, PIPMapper::INTERF_CO_BOOSTING);
		exp->add_var(var_id);
	}
	
	lp.add_equality(exp, 0);
}

void lp_pip_constraint_7(Task& ti, TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, PIPMapper& vars)
{
	uint p_num = processors.get_processor_num();

	if(p_num >= ti.get_id())
	{
		foreach_task_except(tasks.get_tasks(), ti, tx)
		{
			LinearExpression *exp = new LinearExpression();

			uint var_id;
			uint x = tx->get_id();
				
			var_id = vars.lookup(x, 0, 0, PIPMapper::INTERF_REGULAR);
			exp->add_var(var_id);

			var_id = vars.lookup(x, 0, 0, PIPMapper::INTERF_CO_BOOSTING);
			exp->add_var(var_id);

			var_id = vars.lookup(x, 0, 0, PIPMapper::INTERF_STALLING);
			exp->add_var(var_id);
		
			lp_pip_indirected_blocking(ti, *tx, tasks, vars, exp);

			lp_pip_preemption_blocking(ti, *tx, tasks, vars, exp);

			lp.add_equality(exp, 0);
		}
	}
}

void lp_pip_constraint_8(Task& ti, TaskSet& tasks, ResourceSet& resources, LinearProgram& lp, PIPMapper& vars)
{
	foreach_lower_priority_task(tasks.get_tasks(), ti, tx)
	{
		uint x = tx->get_id();
		foreach(tx->get_requests(), request)
		{
			uint q = request->get_resource_id();
			uint N_i_q;
			if(ti.is_request_exist(q))
				N_i_q = ti.get_request_by_id(q).get_num_requests();
			else
				N_i_q = 0;

			LinearExpression *exp = new LinearExpression();
			foreach_request_instance(ti, *tx, q, v)
			{
				uint var_id;
				
				var_id = vars.lookup(x, q, v, PIPMapper::BLOCKING_DIRECT);
				exp->add_var(var_id);
			}

			lp.add_inequality(exp, N_i_q);		
		}
	}
}

void lp_pip_constraint_9(Task& ti, TaskSet& tasks, ProcessorSet& processors, ResourceSet& resources, LinearProgram& lp, PIPMapper& vars)
{

	foreach_higher_priority_task(tasks.get_tasks(), ti, tx)
	{
		uint x = tx->get_id();
		foreach(resources.get_resources(), resource)
		{
			
			uint q = resource->get_resource_id();
			uint N_i_q;
			ulong W_i_q;
			
			if(ti.is_request_exist(q))
			{

				N_i_q = ti.get_request_by_id(q).get_num_requests();

				W_i_q = wait_time_bound(ti, q, tasks, processors, resources);
				if(MAX_LONG == W_i_q)
				{
					continue;
				}

			}
			else
			{
				N_i_q = 0;
				W_i_q = 0;
			}
			
			if(tx->is_request_exist(q))
			{
				LinearExpression *exp = new LinearExpression();

				uint N_x_q = tx->get_request_by_id(q).get_num_requests();

				ulong bound = N_i_q*(tx->get_max_job_num(W_i_q))*N_x_q;

				foreach_request_instance(ti, *tx, q, v)
				{
					uint var_id;

					var_id = vars.lookup(x, q, v, PIPMapper::BLOCKING_DIRECT);
					exp->add_var(var_id);
				}
				
				lp.add_inequality(exp, bound);
			}
			
		}
	}

}

void lp_pip_constraint_10(Task& ti, TaskSet& tasks, ResourceSet& resources, LinearProgram& lp, PIPMapper& vars)
{
	LinearExpression *exp = new LinearExpression();

	foreach_lower_priority_task(tasks.get_tasks(), ti, tx)
	{
		uint x = tx->get_id();
		uint var_id;

		var_id = vars.lookup(x, 0, 0, PIPMapper::INTERF_STALLING);
		exp->add_var(var_id);
	}
	lp.add_equality(exp, 0);
}

void lp_pip_constraint_11(Task& ti, TaskSet& tasks, ResourceSet& resources, LinearProgram& lp, PIPMapper& vars)
{
	ulong bound = 0;
	ulong R_i = ti.get_response_time();

	foreach(resources.get_resources(), resource)
	{
		LinearExpression *exp = new LinearExpression();
		uint q = resource->get_resource_id();

		foreach_lower_priority_task(tasks.get_tasks(), ti, tx)
		{
			uint x = tx->get_id();
			if(tx->is_request_exist(q))
			{
				uint x = tx->get_id();
				
				foreach_request_instance(ti, *tx, q, v)
				{
					uint var_id;

					var_id = vars.lookup(x, q, v, PIPMapper::BLOCKING_INDIRECT);
					exp->add_var(var_id);

					var_id = vars.lookup(x, q, v, PIPMapper::BLOCKING_PREEMPT);
					exp->add_var(var_id);
				}
			}
			else
			{
				//delete exp;
				continue;
			}
		}

		ulong bound = 0;

		foreach_higher_priority_task(tasks.get_tasks(), ti, th)
		{
			bound += th->get_max_request_num(q, R_i);
		}

		lp.add_inequality(exp, bound);
	}
}




















