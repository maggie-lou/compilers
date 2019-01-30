#pragma once

#include <L2.h>

using namespace std;
namespace L2{

  void generate_in_out_sets(Program p);
  void get_in_out_sets(Program p, vector<vector<string>> &in,
                      vector<vector<string>> &out, vector<vector<string>> &kill);

}
