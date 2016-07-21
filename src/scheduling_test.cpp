#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <unistd.h>
#include "tasks.h"
#include "schedulability_test.h"
#include "processors.h"
#include "mgl_chart.h"
#include "xml.h"

#define MAX_LEN 100
#define MAX_METHOD 8


using namespace std;

void requests_gen();
string output_filename(int lambda, double step, int p_num, Range u_range, Range p_range);
string get_method_name(Test_Attribute ta);
void Export_Chart(const char* path, const char* title, double min, double max, double step, const char** names, int n, ...);

int main(int argc,char** argv)
{	
	Int_Set lambdas, p_num, methods;
	Double_Set steps;
	Range_Set p_ranges, u_ranges, d_ranges;
	Test_Attribute_Set test_attributes;
	uint exp_times;
	Result_Set results[MAX_METHOD];
	Chart chart;

	XML::LoadFile("config.xml");

	if(0 == access(string("results").data(), 0))
		printf("results folder exsists.\n");
	else
	{
		printf("results folder does not exsist.\n");
		if(0 == mkdir(string("results").data(), S_IRWXU))
			printf("results folder has been created.\n");
		else
			return 0;
	}

	//scheduling parameter
	XML::get_method(&test_attributes);
	exp_times = XML::get_experiment_times();
	XML::get_lambda(&lambdas);
	XML::get_processor_num(&p_num);
	XML::get_period_range(&p_ranges);
	XML::get_deadline_propotion(&d_ranges);
	XML::get_utilization_range(&u_ranges);
	XML::get_step(&steps);	

	//resource parameter
	Int_Set resource_num, rrns;
	Double_Set rrps, tlfs;
	Range_Set rrrs;
	XML::get_resource_num(&resource_num);
	XML::get_resource_request_probability(&rrps);
	XML::get_resource_request_num(&rrns);
	XML::get_resource_request_range(&rrrs);
	XML::get_total_len_factor(&tlfs);

	string file_name = "results/" + output_filename(lambdas[0], steps[0], p_num[0], u_ranges[0], p_ranges[0]) + ".csv";
	ofstream output_file(file_name);

	output_file<<"Lambda:"<<lambdas[0]<<",";					
	output_file<<" processor number:"<<p_num[0]<<",";
	output_file<<" step:"<<steps[0]<<",";
	output_file<<" utilization range:["<<u_ranges[0].min<<"-"<<u_ranges[0].max<<"],";
	output_file<<setprecision(0)<<" period range:["<<p_ranges[0].min<<"-"<<p_ranges[0].max<<"]\n"<<setprecision(3);
	output_file<<"Utilization,";
	for(uint i = 0; i < test_attributes.size(); i++)
	{
		output_file<<get_method_name(test_attributes[i])<<" ratio,";
	}
	output_file<<"\n";

	double utilization = u_ranges[0].min;

	do
	{	
		Result result;
		cout<<"Utilization:"<<utilization<<endl;
		int success[MAX_METHOD] = {0, 0, 0, 0, 0, 0};
		for(int i = 0; i < exp_times; i++)
		{
			TaskSet taskset = TaskSet();
			ResourceSet resourceset = ResourceSet();
			ProcessorSet processorset = ProcessorSet(p_num[0]);
			resource_gen(&resourceset, resource_num[0]);
			tast_gen(taskset, resourceset, lambdas[0], p_ranges[0], d_ranges[0], utilization, rrps[0], rrns[0], rrrs[0], tlfs[0]);	
			for(uint i = 0; i < test_attributes.size(); i++)
			{
				taskset.init();
				processorset.init();
				if(is_schedulable(taskset, processorset, resourceset, test_attributes[i].test_method, test_attributes[i].test_type, 0))
				{	
					success[i]++;
				}
			}
			result.x = taskset.get_utilization_sum().get_d();
		}
		for(uint i = 0; i < test_attributes.size(); i++)
		{
			fraction_t ratio(success[i], exp_times);
			result.y = ratio.get_d();
			results[i].push_back(result);
		}
//output to csv file		
		output_file<<utilization<<",";
		for(uint i = 0; i < test_attributes.size(); i++)
		{
			output_file<<results[i][results[i].size()-1].y<<",";
		}
		output_file<<"\n";
		output_file.flush();
		utilization += steps[0];
	}
	while(utilization < u_ranges[0].max || fabs(u_ranges[0].max - utilization) < EPS);

	for(uint i = 0; i < test_attributes.size(); i++)
	{
		string test_name = get_method_name(test_attributes[i]);
		chart.AddData(test_name, results[i]);
	}

	output_file.close();
	
	string png_name = "results/" + output_filename(lambdas[0], steps[0], p_num[0], u_ranges[0], p_ranges[0]) + ".png";

	chart.SetGraphSize(1280,800);
	chart.SetGraphQual(3);
	chart.ExportPNG(png_name.data(), "", u_ranges[0].min, results[0][results[0].size()-1].x);

	return 0;
}

string output_filename(int lambda, double step, int p_num, Range u_range, Range p_range)
{
	stringstream buf;
	buf<<"l:"<<lambda<<"-"<<"s:"<<step<<"-"<<"P:"<<p_num<<"-"<<fixed<<setprecision(0)<<"u:["<<u_range.min<<","<<u_range.max<<"]-"<<"p:["<<p_range.min<<","<<p_range.max<<"]";
	return buf.str();
}

string get_method_name(Test_Attribute ta)
{
	string name;
	switch(ta.test_method)
	{
		default:
			name = "P-EDF";
			break;
		case 0:
			name =  "P-EDF";
			break;
		case 1:
			name =  "BCL-FTP";
			break;
		case 2:
			name =  "BCL-EDF";
			break;
		case 3:
			name = "WF-DM";
			break;
		case 4:
			name = "WF-EDF";
			break;
		case 5:
			name = "RTA-GFP";
			break;
	}
	if("" != ta.remark)
		return name + "-" + ta.remark;
	else
		return name;
}




