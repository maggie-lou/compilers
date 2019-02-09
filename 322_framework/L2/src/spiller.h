#pragma once

#include <L2.h>

namespace L2{

  void print_spill(Program p);
  Function* spill(Function* f, string var_name, string prefix);

}
