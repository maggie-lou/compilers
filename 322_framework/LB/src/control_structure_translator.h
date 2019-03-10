#pragma once

#include <map>
#include <string>

#include <LB.h>

using namespace std;

namespace LB {

  vector<string> generate_branch_code(Instruction* i, string condition_op1, string condition_op2, string op, string label1, string label2, string &longest_var, int64_t &var_count);

  map<Instruction*, Instruction_while*> map_instructions_to_loop(vector<Instruction*> &instructions, map<string, Instruction_while*> start_label_to_while_map, map<string, Instruction*> end_label_to_while_map, map<Instruction*, string> &while_to_cond_label_map);

  vector<Instruction*> add_cond_labels_while(vector<Instruction*> instructions, map<Instruction*, string> while_to_cond_label_map, string &longest_var, int64_t &var_count);

}
