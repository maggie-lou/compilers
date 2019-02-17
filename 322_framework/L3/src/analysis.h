#pragma once

#include <L3.h>

using namespace std;
namespace L3{

  void generate_and_print_in_out_sets(Program p);
  void get_in_out_sets(Function* f, vector<vector<string>> &in, vector<vector<string>> &out);

}
