#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>

#include <utils.h>
#include <LB.h>
#include <control_structure_translator.h>

using namespace std;

namespace LB {

  vector<string> translate_if_to_LA(Instruction_if* i, string &longest_var, int64_t &var_count) {
    vector<string> LA_code;
    string temp_condition_var = i->generate_unique_var_name(longest_var, var_count);
    string condition_declaration = "\tint64 " + temp_condition_var + "\n";
    string condition = "\t" + temp_condition_var + " <- " + i->t1->to_string() + " " + i->op + " " + i->t2->to_string() + "\n";
    string jump = "\t br " + temp_condition_var + " " + i->label1->to_string() + " " + i->label2->to_string() + "\n";

    LA_code.push_back(condition_declaration);
    LA_code.push_back(condition);
    LA_code.push_back(jump);

    return LA_code;
  }

  map<Instruction*, Instruction*> map_instructions_to_loop(vector<Instruction*> instructions, map<string, Instruction*> start_label_to_while_map, map<string, Instruction*> end_label_to_while_map) {
    map<Instruction*, Instruction*> instruction_to_loop_map;
    set<Instruction*> while_seen;
    stack<Instruction*> loop_stack;

    for (Instruction* i : instructions) {
      // Map instructions to loop
      bool within_loop = loop_stack.size() != 0;
      if (within_loop) {
        instruction_to_loop_map.insert(pair <Instruction*, Instruction*> (i, loop_stack.top()));
      }

      if (Instruction_while* while_i = dynamic_cast<Instruction_while*> (i)) {
        if ( while_seen.find(i) == while_seen.end()) {
          // Slides say to add this, I think it's wrong?
          loop_stack.push(i);
          while_seen.insert(i);
          continue;
        }
      }

      if (Instruction_label* label_i = dynamic_cast<Instruction_label*> (i)) {
        // Generate new loop for start label
        if (start_label_to_while_map.find(label_i->label->to_string()) != start_label_to_while_map.end()) {
          Instruction* current_while = start_label_to_while_map[label_i->label->to_string()];
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

  void translate_while_to_LA(Function* f, string &longest_var, int64_t &var_count) {
    map<string, Instruction*> start_label_to_while_map;
    map<string, Instruction*> end_label_to_while_map;
    map<Instruction*, Instruction*> cond_label_to_while_map;

    for (int index = 0; index < f->instructions.size(); index++) {
      if (Instruction_while* while_i = dynamic_cast<Instruction_while*>(f->instructions[index])) {
        // Identify entry and exit point of each while
        start_label_to_while_map.insert( pair< string, Instruction* >( while_i->label1->to_string(), while_i ));
        end_label_to_while_map.insert( pair< string, Instruction* >( while_i->label2->to_string(), while_i ));

        // Add label to jump to while loop
        string label_name = ":" + f->instructions[index]->generate_unique_var_name(longest_var, var_count);
        Label *l = new Label();
        Instruction_label *label_i = new Instruction_label();
        l->name = label_name;
        label_i->label = l;
        f->instructions.insert(f->instructions.begin() + index, label_i);
        index++;
        cond_label_to_while_map.insert( pair <Instruction*, Instruction* > (while_i, label_i));
      }
    }

    map<Instruction*, Instruction*> instruction_to_loop_map = map_instructions_to_loop(f->instructions, start_label_to_while_map, end_label_to_while_map);
  }

}
