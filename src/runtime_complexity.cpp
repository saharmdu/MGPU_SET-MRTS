// Copyright [2018] <Zewei Chen>
// ------By Zewei Chen------
// Email:czwking1991@gmail.com
#include <glpk.h>
#include <iteration-helper.h>
#include <output.h>
#include <param.h>
#include <processors.h>
#include <pthread.h>
#include <time_record.h>
#include <random_gen.h>
#include <resources.h>
#include <sched_test_base.h>
#include <sched_test_factory.h>
#include <solution.h>
#include <tasks.h>
#include <unistd.h>
#include <xml.h>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define MAX_LEN 100
#define MAX_METHOD 8

#define typeof(x) __typeof(x)

using std::cout;
using std::endl;
using std::ifstream;
using std::string;

void getFiles(string path, string dir);
void read_line(string path, vector<string>* files);
vector<Param> get_parameters();

int main(int argc, char** argv) {
  vector<Param> parameters = get_parameters();
  cout << parameters.size() << endl;
  foreach(parameters, param) {
    Result_Set results[MAX_METHOD];
    SchedTestFactory STFactory;
    Output output(*param);
    XML::SaveConfig((output.get_path() + "config.xml").data());
    output.export_param();

#if UNDEF_ABANDON
    GLPKSolution::set_time_limit(TIME_LIMIT_INIT);
#endif

    Random_Gen::uniform_integral_gen(0, 10);
    double utilization = param->u_range.min;

    time_t start, end;
    char time_buf[40];

    start = time(NULL);

    cout << endl << "Strat at:" << ctime_r(&start, time_buf) << endl;

    do {
      Result result;
      cout << "Utilization:" << utilization << endl;
      vector<int> success;
      vector<int> exp;
      vector<int> exc;
      for (uint i = 0; i < param->test_attributes.size(); i++) {
        exp.push_back(0);
        success.push_back(0);
        exc.push_back(0);
      }

      Time_Record timer;
      timer.Record_MS_A();
      // time_t s, e;
      // s = time(NULL);

      for (int i = 0; i < param->exp_times; i++) {
        TaskSet taskset = TaskSet();
        ProcessorSet processorset = ProcessorSet(*param);
        ResourceSet resourceset = ResourceSet();
        resource_gen(&resourceset, *param);
        task_gen_UUnifast_Discard(&taskset, &resourceset, *param, utilization);

        SchedTestBase* schedTest =
                STFactory.createSchedTest(param->test_attributes[0].test_name,
                                          taskset, processorset, resourceset);
        if (NULL == schedTest) {
          return -1;
        }
        if (schedTest->is_schedulable()) {
        } 
      }

      timer.Record_MS_B();
      double ms_time = timer.Record_MS();

      stringstream buf;
      if (0 == strcmp(param->test_attributes[0].rename.data(), ""))
        buf << param->test_attributes[0].test_name;
      else
        buf << param->test_attributes[0].rename;
      buf << "," << utilization << ","  << ms_time/param->exp_times;
      output.append2file("runtime_complexity.csv", buf.str());

      // time(&e);
      // ulong gap = difftime(e, s);
      // uint hour = gap / 3600;
      // uint min = (gap % 3600) / 60;
      // uint sec = (gap % 3600) % 60;

      // cout << hour << "hour " << min << "min " << sec << "sec. " << endl;

      utilization += param->step;
    } while (utilization < param->u_range.max ||
            fabs(param->u_range.max - utilization) < _EPS);
  }

  return 0;
}

void getFiles(string path, string dir) {
  string cmd = "ls " + path + " > " + path + dir;
  system(cmd.data());
}

void read_line(string path, vector<string>* files) {
  string buf;
  ifstream dir(path.data(), ifstream::in);
  getline(dir, buf);
  while (getline(dir, buf)) {
    files->push_back("config/" + buf);
    // cout<<"file name:"<<buf<<endl;
  }
}

vector<Param> get_parameters() {
  vector<Param> parameters;

  XML::LoadFile("config.xml");

  if (0 == access(string("results").data(), 0)) {
    printf("results folder exsists.\n");
  } else {
    printf("results folder does not exsist.\n");
    if (0 == mkdir(string("results").data(), S_IRWXU))
      printf("results folder has been created.\n");
    else
      return parameters;
  }

  // Server info
  const char* ip = XML::get_server_ip();
  uint port = XML::get_server_port();

  Int_Set lambdas, p_nums;
  Double_Set steps, ratio;
  Range_Set p_ranges, u_ranges, d_ranges;
  Test_Attribute_Set test_attributes;
  uint exp_times;

  // scheduling parameter
  XML::get_method(&test_attributes);
  exp_times = XML::get_experiment_times();
  XML::get_lambda(&lambdas);
  XML::get_processor_num(&p_nums);
  XML::get_period_range(&p_ranges);
  XML::get_deadline_propotion(&d_ranges);
  XML::get_utilization_range(&u_ranges);
  XML::get_step(&steps);
  XML::get_doubles(&ratio, "processor_ratio");

  // resource parameter
  Int_Set resource_nums, rrns, mcsns;
  Double_Set rrps, tlfs;
  Range_Set rrrs;
  XML::get_resource_num(&resource_nums);
  XML::get_resource_request_probability(&rrps);
  XML::get_resource_request_num(&rrns);
  XML::get_resource_request_range(&rrrs);
  XML::get_total_len_factor(&tlfs);
  XML::get_integers(&mcsns, "mcsn");

  // graph parameters
  Range_Set job_num_ranges;
  Range_Set arc_num_ranges;
  Int_Set is_cyclics;
  Int_Set max_indegrees;
  Int_Set max_outdegrees;
  Double_Set para_probs, cond_probs, arc_densities;
  Int_Set max_para_jobs, max_cond_branches;

  XML::get_ranges(&job_num_ranges, "dag_job_num_range");
  XML::get_ranges(&arc_num_ranges, "dag_arc_num_range");
  XML::get_integers(&is_cyclics, "is_cyclic");
  XML::get_integers(&max_indegrees, "max_indegree");
  XML::get_integers(&max_outdegrees, "max_outdegree");
  XML::get_doubles(&para_probs, "paralleled_probability");
  XML::get_doubles(&cond_probs, "conditional_probability");
  XML::get_doubles(&arc_densities, "dag_arc_density");
  XML::get_integers(&max_para_jobs, "max_paralleled_job");
  XML::get_integers(&max_cond_branches, "max_conditional_branch");

  foreach(lambdas, lambda)
    foreach(p_nums, p_num)
      foreach(steps, step)
        foreach(p_ranges, p_range)
          foreach(u_ranges, u_range)
            foreach(d_ranges, d_range)
              foreach(resource_nums, resource_num)
                foreach(rrns, rrn)
                  foreach(mcsns, mcsn)
                    foreach(rrps, rrp)
                      foreach(tlfs, tlf)
                        foreach(rrrs, rrr)
                          foreach(job_num_ranges, job_num_range)
                            foreach(arc_num_ranges, arc_num_range)
                              foreach(max_indegrees, max_indegree)
                                foreach(max_outdegrees, max_outdegree)
                                  foreach(para_probs, para_prob)
                                    foreach(cond_probs, cond_prob)
                                      foreach(arc_densities, arc_density)
                                        foreach(max_para_jobs, max_para_job)
                                          foreach(max_cond_branches,
                                                   max_cond_branch) {
                                            Param param;
                                            // set parameters
                                            param.id = parameters.size();
                                            param.server_ip = ip;
                                            param.port = port;
                                            param.lambda = *lambda;
                                            param.p_num = *p_num;
                                            param.ratio = ratio;
                                            param.step = *step;
                                            param.p_range = *p_range;
                                            param.u_range = *u_range;
                                            param.d_range = *d_range;
                                            param.test_attributes =
                                                test_attributes;
                                            param.exp_times = exp_times;
                                            param.resource_num = *resource_num;
                                            param.mcsn = *mcsn;
                                            param.rrn = *rrn;
                                            param.rrp = *rrp;
                                            param.tlf = *tlf;
                                            param.rrr = *rrr;

                                            param.job_num_range =
                                                *job_num_range;
                                            param.arc_num_range =
                                                *arc_num_range;

                                            if (0 == is_cyclics[0])
                                              param.is_cyclic = false;
                                            else
                                              param.is_cyclic = true;

                                            param.max_indegree = *max_indegree;
                                            param.max_outdegree =
                                                *max_outdegree;
                                            param.para_prob = *para_prob;
                                            param.cond_prob = *cond_prob;
                                            param.arc_density = *arc_density;
                                            param.max_para_job = *max_para_job;
                                            param.max_cond_branch =
                                                *max_cond_branch;

                                            parameters.push_back(param);
                                          }
  cout << "param num:" << parameters.size() << endl;
  return parameters;
}
