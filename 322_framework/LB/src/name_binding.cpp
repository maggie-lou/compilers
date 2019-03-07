#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>

#include <utils.h>
#include <LB.h>

using namespace std;

namespace LB {

  void binding_name(Program p){
    for (Function* f : p.functions){
      for (Instruction* i : f->instructions){
        i->name_binding(p.longest_var, p.var_count);
      }
    }
  }
}
