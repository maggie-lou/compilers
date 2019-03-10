#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <set>

#include <utils.h>
#include <LB.h>
#include <control_structure_translator.h>

using namespace std;

namespace LB {

  vector<string> generate_branch_code(Instruction* i, string condition_op1, string condition_op2, string op, string label1, string label2, string &longest_var, int64_t &var_count) {
    vector<string> LA_code;
    string temp_condition_var = i->generate_unique_var_name(longest_var, var_count);
    string condition_declaration = "\tint64 " + temp_condition_var + "\n";
    string condition = "\t" + temp_condition_var + " <- " + condition_op1 + " " + op + " " + condition_op2 + "\n";
    string jump = "\t br " + temp_condition_var + " " + label1 + " " + label2 + "\n";

    LA_code.push_back(condition_declaration);
    LA_code.push_back(condition);
    LA_code.push_back(jump);

    return LA_code;
  }

  vector<Instruction*> add_cond_labels_while(vector<Instruction*> instructions, map<Instruction*, string> while_to_cond_label_map, string &longest_var, int64_t &var_count) {
    for (int index = 0; index < instructions.size(); index++) {
      Instruction* i = instructions[index];

      if (Instruction_while* while_i = dynamic_cast<Instruction_while*> (i)) {
        // Add label to jump to while loop
        string label_name = ":" + i->generate_unique_var_name(longest_var, var_count);
        Instruction_label* label_i = new Instruction_label();
        Label* l = new Label();
        l->name = label_name;
        label_i->label = l;

        while_to_cond_label_map.insert( pair <Instruction*, string > (while_i, label_name));
        instructions.insert(instructions.begin() + index, label_i);
        index++;
      }
    }

    return instructions;
  }

  map<Instruction*, Instruction_while*> map_instructions_to_loop(vector<Instruction*> &instructions, map<string, Instruction_while*> start_label_to_while_map, map<string, Instruction*> end_label_to_while_map, map<Instruction*, string> &while_to_cond_label_map) {
    map<Instruction*, Instruction_while*> instruction_to_loop_map;
    set<Instruction_while*> while_seen;
    stack<Instruction_while*> loop_stack;

    for (int index = 0; index < instructions.size(); index++) {
      Instruction* i = instructions[index];

      // Map instructions to loop
      bool within_loop = loop_stack.size() != 0;
      if (within_loop) {
        instruction_to_loop_map.insert(pair <Instruction*, Instruction_while*> (i, loop_stack.top()));
      }

      if (Instruction_while* while_i = dynamic_cast<Instruction_while*> (i)) {
        if ( while_seen.find(while_i) == while_seen.end()) {
          // Slides say to add this, I think it's wrong?
          loop_stack.push(while_i);
          while_seen.insert(while_i);
          continue;
        }
      }

      if (Instruction_label* label_i = dynamic_cast<Instruction_label*> (i)) {
        // Generate new loop for start label
        if (start_label_to_while_map.find(label_i->label->to_string()) != start_label_to_while_map.end()) {
          Instruction_while* current_while = start_label_to_while_map[label_i->label->to_string()];
          if (while_seen.find(current_while) == while_seen.end()) {
            loop_stack.push(current_while);
            continue;
          }
        }

        // End current loop for end label
        if (end_label_to_while_map.find(label_i->label->to_string()) != end_label_to_while_map.end()) {
          loop_stack.pop();
        }
      }
    }

    return instruction_to_loop_map;
  }
}
