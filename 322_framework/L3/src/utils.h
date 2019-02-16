#pragma once

#include <L3.h>

using namespace std;

namespace L3{

  extern vector<string> argument_registers;

  bool is_int(std::string s);
  bool contains(vector<string> v, string s);
}
