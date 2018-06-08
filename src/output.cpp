// Copyright [2016] <Zewei Chen>
// ------By Zewei Chen------
// Email:czwking1991@gmail.com
#include <output.h>
#include <iteration-helper.h>
#include <math-helper.h>

Output::Output(const char* path) {
  this->path = path;
  chart.SetGraphSize(800, 600);
  chart.SetGraphQual(3);
}

Output::Output(Param param) {
  this->param = param;
  if (0 == access(string("results/" + output_filename()).data(), 0)) {
    int suffix = 0;
    printf("result folder exsists.\n");
    do {
      suffix++;
    } while (0 == access(string("results/" + output_filename() + "-" +
                                to_string(suffix))
                             .data(),
                         0));
    if (0 ==
        mkdir(string("results/" + output_filename() + "-" + to_string(suffix))
                  .data(),
              S_IRWXU))

      this->path =
          "results/" + output_filename() + "-" + to_string(suffix) + "/";
  } else {
    printf("result folder does not exsist.\n");
    if (0 == mkdir(string("results/" + output_filename()).data(), S_IRWXU))
      printf("result folder has been created.\n");
    this->path = "results/" + output_filename() + "/";
    printf("result folder has been created.\n");
  }

  chart.SetGraphSize(1280, 640);
  chart.SetGraphQual(3);
}

string Output::get_path() { return path; }

SchedResultSet& Output::get_results() { return srs; }

bool Output::add_result(string test_name, string line_style, double utilization,
                        uint e_num, uint s_num) {
  SchedResult& sr = srs.get_sched_result(test_name, line_style);
  if (sr.get_result_by_utilization(utilization).exp_num + e_num <=
      param.exp_times) {
    sr.insert_result(utilization, e_num, s_num);
    return true;
  }
  return false;
}

uint Output::get_exp_time_by_utilization(double utilization) {
  if (0 == srs.size()) return 0;

  Result result =
      srs.get_sched_result_set()[0].get_result_by_utilization(utilization);
  return result.exp_num;
}

string Output::output_filename() {
  stringstream buf;
  buf << "id[" << param.id << "]-l[" << param.lambda << "]-"
      << "P[" << param.p_num << "]-"
      << "rn[" << param.resource_num << "]-"
      << "rrn[" << param.rrn << "]-"
      << "rrp[" << param.rrp << "]-"
      << "rrr[" << param.rrr.min << "," << param.rrr.max << "]";
  return buf.str();
}

string Output::get_method_name(Test_Attribute ta) {
  string name;
  name = ta.test_name;

  if (0 == strcmp(ta.remark.data(), ""))
    return name;
  else
    return name + "-" + ta.remark;
}

// output to console
void Output::proceeding() {}

// void Output::proceeding(string test_name, double utilization, uint e_num,
//                         uint s_num) {}

void Output::finish() {}

// export to csv
void Output::export_param() {
  string file_name = path + "parameters.txt";
  ofstream output_file(file_name);

  output_file << "Lambda: " << param.lambda << "\n";
  output_file << "Processor num: " << param.p_num << "\n";
  output_file << "Period range: [" << param.p_range.min << "-"
              << param.p_range.max << "]"
              << "\n";
  output_file << "Utilization range: [" << param.u_range.min << "-"
              << param.u_range.max << "]"
              << "\n";
  output_file << "Deadline proportion range: [" << param.d_range.min << "-"
              << param.d_range.max << "]"
              << "\n";
  output_file << "Resource num: " << param.resource_num << "\n";
  output_file << "Max critical section num: " << param.mcsn << "\n";
  output_file << "Resource request num: " << param.rrn << "\n";
  output_file << "Resource request probability: " << param.rrp << "\n";
  output_file << "Total length factor: " << param.tlf << "\n";
  output_file << "Resource request range: [" << param.rrr.min << "-"
              << param.rrr.max << "]"
              << "\n";

  output_file.flush();
  output_file.close();
}
void Output::export_csv() {
  string file_name = path + "result.csv";
  ofstream output_file(file_name);

  output_file << "Lambda:" << param.lambda << ",";
  output_file << " processor number:" << param.p_num << ",";
  output_file << " step:" << param.step << ",";
  output_file << " utilization range:[" << param.u_range.min << "-"
              << param.u_range.max << "],";
  output_file << setprecision(0) << " period range:[" << param.p_range.min
              << "-" << param.p_range.max << "]\n"
              << setprecision(3);
  output_file << "Utilization,";
  for (uint i = 0; i < param.test_attributes.size(); i++) {
    output_file << get_method_name(param.test_attributes[i]) << " ratio,";
  }
  output_file << "\n";
  /*
          for(uint i = 0; i < result_sets[0].size(); i++)
          {
                  output_file<<result_sets[0][i].x<<",";
                  for(uint j = 0; j < result_sets.size(); j++)
                  {
                          output_file<<result_sets[j][i].y<<",";
                  }
                  output_file<<"\n";
          }
  */

  for (double i = param.u_range.min; i - param.u_range.max < _EPS;
       i += param.step) {
    output_file << i << ",";
    foreach(srs.get_sched_result_set(), sched_result) {
      if (0 == sched_result->get_result_by_utilization(i).exp_num) {
        output_file << ",";
      } else {
        double ratio = sched_result->get_result_by_utilization(i).success_num;
        ratio /= sched_result->get_result_by_utilization(i).exp_num;
        output_file << ratio << ",";
      }
    }
    output_file << "\n";
  }

  output_file.flush();
  output_file.close();
}

void Output::export_table_head() {
  string file_name = path + "result-step-by-step.csv";
  ofstream output_file(file_name);

  output_file << "Lambda:" << param.lambda << ",";
  output_file << "processor number:" << param.p_num << ",";
  output_file << "step:" << param.step << ",";
  output_file << "utilization range:[" << param.u_range.min << "-"
              << param.u_range.max << "],";
  output_file << setprecision(0) << "period range:[" << param.p_range.min << "-"
              << param.p_range.max << "]\n"
              << setprecision(3);
  output_file << "Scheduling test method:,";
  for (uint i = 0; i < param.test_attributes.size(); i++) {
    output_file << get_method_name(param.test_attributes[i]) << ","
                << ",,";
  }
  output_file << "\n";
  output_file << "Utilization,";
  for (uint i = 0; i < param.test_attributes.size(); i++) {
    output_file << "experiment times,success times,success ratio,";
  }
  output_file << "\n";
  output_file.flush();
  output_file.close();
}

void Output::export_result_append(double utilization) {
  string file_name = path + "result-step-by-step.csv";
  if (0 != access(file_name.data(), 0)) {
    export_table_head();
  }
  ofstream output_file(file_name, ofstream::app);

  /*
          uint last_index = result_sets[0].size() - 1;
          output_file<<result_sets[0][last_index].x<<",";
          for(uint i = 0; i < param.test_attributes.size(); i++)
          {
                  Result result = result_sets[i][last_index];
                  output_file<<result.exp_num<<",";
                  output_file<<result.success_num<<",";
                  output_file<<result.y<<",";
          }
          output_file<<"\n";
  */

  output_file << utilization << ",";
  foreach(srs.get_sched_result_set(), sched_result) {
    uint e_num = sched_result->get_result_by_utilization(utilization).exp_num;
    uint s_num =
        sched_result->get_result_by_utilization(utilization).success_num;
    if (0 == e_num) {
      output_file << ",,,";
    } else {
      double ratio = s_num;
      ratio /= e_num;
      output_file << e_num << ",";
      output_file << s_num << ",";
      output_file << ratio << ",";
    }
  }
  output_file << "\n";

  output_file.flush();
  output_file.close();
}

void Output::append2file(string flie_name, string buffer) {
  string file_name = path + flie_name;

  ofstream output_file(file_name, ofstream::app);

  output_file << buffer << "\n";
  output_file.flush();
  output_file.close();
}

// export to graph
void Output::SetGraphSize(int width, int height) {
  chart.SetGraphSize(width, height);
}

void Output::SetGraphQual(int quality) { chart.SetGraphQual(quality); }

void Output::Export(int format) {
  string temp, file_name = path + "result";
  /*
          for(uint i = 0; i < get_sets_num(); i++)
          {
                  chart.AddData(get_method_name(param.test_attributes[i]),
     result_sets[i]);
          }
  */
  chart.AddData(srs);

  if (0x0f & format) {
    chart.ExportLineChart(file_name, "", param.u_range.min, param.u_range.max,
                          param.step, format);
  }
  if (0x10 & format) {
    temp = file_name + ".json";
    chart.ExportJSON(temp);
  }
}
