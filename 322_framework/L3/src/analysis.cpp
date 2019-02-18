#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>
#include <iterator>

#include <analysis.h>
#include <utils.h>
#include <functional>

using namespace std;

namespace L3{

  vector<string> get_union(vector<string> v1, vector<string> v2)  {
    vector<string> union_vec;
    set_union(v1.begin(), v1.end(), v2.begin(), v2.end(),
              back_inserter(union_vec));
    set<string> union_set(union_vec.begin(), union_vec.end());
    vector<string> ans(union_set.begin(), union_set.end());
    return ans;
  }

  vector<string> get_difference(vector<string> v1, vector<string> v2)  {
    vector<string> diff_vec;
    set_difference(v1.begin(), v1.end(), v2.begin(), v2.end(),
              inserter(diff_vec, diff_vec.begin()));
    set<string> diff_set(diff_vec.begin(), diff_vec.end());
    vector<string> ans(diff_set.begin(), diff_set.end());
    return ans;
  }

  int find_line_num_label(string label, vector<Instruction*> instructions) {
    for (int i = 0; i<instructions.size(); i++) {
      Instruction_label* label_instruction = dynamic_cast<Instruction_label*>(instructions.at(i));
      if (label_instruction) {
        if (label == label_instruction->label->name) {
          return i;
        }
      }
    }
    return -1;
  }

  vector<string> generate_out_set(int current_instruction_index, vector<Instruction*> instructions, vector<vector<string>> in) {
    vector<string> output;

    if (auto is_goto = dynamic_cast<Instruction_goto*>(instructions.at(current_instruction_index))) {
      auto line_num_label = find_line_num_label(is_goto->label->name, instructions);

      if (line_num_label < instructions.size() - 1) {
        auto in_set = in[line_num_label+1];
        output.insert(output.end(), in_set.begin(), in_set.end());
      }
    } else if (auto is_cond_branch = dynamic_cast<Instruction_jump*>(instructions.at(current_instruction_index))) {
      auto line_num_label = find_line_num_label(is_cond_branch->label->name, instructions);

      if (line_num_label < instructions.size() - 1) {
        auto in_set = in[line_num_label+1];
        output.insert(output.end(), in_set.begin(), in_set.end());
      }

      // Fallthrough
      if (current_instruction_index < instructions.size() - 1) {
        auto in_set = in[current_instruction_index+1];
        output.insert(output.end(), in_set.begin(), in_set.end());
      }
    }  else if (auto is_return = dynamic_cast<Instruction_ret*>(instructions.at(current_instruction_index))) {
      // Do nothing - return instructions have no out set
    } else  {
      if (current_instruction_index < instructions.size() - 1) {
        output = in[current_instruction_index+1];
      }
    }
    return output;
  }

  void get_in_out_sets(Function* f, vector<vector<string>> &in, vector<vector<string>> &out){
    //cout <<"Generating in out sets "<<endl;
    auto instructions = f->instructions;
    vector<vector<string>> gen(instructions.size());
    bool changed = false;
    vector<vector<string>> kill(instructions.size());

    for (int j=0; j<instructions.size(); j++) {
      auto current_i = instructions[j];
     // cout << "About to generate read" << endl;
      auto temp_gen = current_i->generate_read();
      //cout << "About to generate defined" << endl;
      auto temp_kill = current_i->generate_defined();
      gen[j] = temp_gen;
      kill[j] = temp_kill;
      in[j] = {};
      out[j] = {};
    }
    do {
      changed = false;
      for (int j=instructions.size()-1; j>=0; j--) {
        vector<string> old_in = in[j];
        vector<string> old_out = out[j];

        vector<string> diff = get_difference(out[j], kill[j]);
        in[j] = get_union(gen[j], diff);
        out[j] = generate_out_set(j, instructions, in);
        vector<string> diff_in = get_difference(in[j], old_in);
        vector<string> diff_out = get_difference(out[j], old_out);

        changed = changed || !diff_in.empty() || !diff_out.empty();
      }
    } while (changed);
    return;
  }
}
