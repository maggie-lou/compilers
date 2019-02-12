#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <algorithm>
#include <iterator>

#include <spiller.h>
#include <utils.h>
#include <functional>

using namespace std;

namespace L2{
  Assignment* create_assignment(Item* s, Item* d, string op){
    Assignment* assignment = new Assignment();
    assignment->op = op;
    assignment->d = d;
    assignment->s = s;
    return assignment;
  }

  Var_item* create_var(string prefix, int64_t counter){
    Var_item* var = new Var_item();
    var->var_name = prefix + to_string(counter);
    return var;
  }

  // Return value indicates whether the intended variable was in the instruction, to know whether stack reads/writes are necessary
  bool check_replace_var(Item* &to_check, string var_name, Var_item* new_var){
    bool is_var = false;
    if (Var_item* var = dynamic_cast<Var_item*>(to_check)){
      if (var->var_name == var_name){
        to_check = new_var;
        is_var = true;
      }
      // Intended variable was already replaced
      // Implies that intended variable was in instruction originally - still require a stack read/write
      if (var->var_name == new_var->var_name){
        is_var = true;
      }
    }
    return is_var;
  }

  Function* spill(Function* f, string var_name, string prefix){
    Function* new_f = new Function();
    int64_t locals = f->locals;
    new_f->name = f->name;
    new_f->arguments = f->arguments;
    Address_item* stack_address = new Address_item();
    stack_address->x = new Register_item("rsp");
    stack_address->offset = locals * 8;
    int64_t counter = 0;

    auto instructions = f->instructions;
    for (Instruction* i : instructions){
      vector<reference_wrapper<Item*>> gen = i->generate_gen();
      vector<reference_wrapper<Item*>> kill = i->generate_kill();
      bool gen_has_var = false;
      bool kill_has_var = false;
      Var_item* new_var = create_var(prefix, counter);

      for (Item*& g : gen){
        gen_has_var = gen_has_var || check_replace_var(g, var_name, new_var);
      }
      for (Item*& k : kill){
        kill_has_var = kill_has_var || check_replace_var(k, var_name, new_var);
      }
      if (gen_has_var){
        new_f->instructions.push_back(create_assignment(stack_address, new_var, "<-"));
      }
      new_f->instructions.push_back(i);
      if (kill_has_var){
        new_f->instructions.push_back(create_assignment(new_var, stack_address, "<-"));
      }
      if (gen_has_var || kill_has_var){
        counter++;
      }
    }

    if (counter==0) {
      new_f->locals = locals;
    } else {
      new_f->locals = locals + 1;
    }
    return new_f;
  }

  void print_spill(Program p){
    Function* f = p.functions.front();
    Function* new_f = spill(f, f->var_name, f->prefix);
    L2::print_function(new_f);
  }
}
