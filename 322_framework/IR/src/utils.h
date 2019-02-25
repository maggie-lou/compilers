#pragma once

#include <IR.h>
#include <unordered_map>
#include <stack>

using namespace std;

namespace IR{

  bool is_int(std::string s);
  bool contains(vector<string> v, string s);
  IR::Variable_type str_to_variable_type(string s);
}
