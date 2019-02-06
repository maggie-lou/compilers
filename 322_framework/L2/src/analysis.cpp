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

namespace L2{

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
      Label_instruction* label_instruction = dynamic_cast<Label_instruction*>(instructions.at(i));
      if (label_instruction) {
        if (label == label_instruction->label) {
          return i;
        }
      }
    }
    return -1;
  }

  vector<string> generate_out_set(int current_instruction_index, vector<Instruction*> instructions, vector<vector<string>> in) {
    vector<string> output;

    if (auto is_cjump = dynamic_cast<Cjump*>(instructions.at(current_instruction_index))) {
      auto line_num_label1 = find_line_num_label(is_cjump->label1, instructions);
      auto line_num_label2 = find_line_num_label(is_cjump->label2, instructions);

      if (line_num_label1 < instructions.size() - 1) {
        auto in_set = in[line_num_label1+1];
        output.insert(output.end(), in_set.begin(), in_set.end());
      }
      if (line_num_label2 < instructions.size() - 1) {
        auto in_set = in[line_num_label2+1];
        output.insert(output.end(), in_set.begin(), in_set.end());
      }
    } else if (auto is_cjump_fallthrough = dynamic_cast<Cjump_fallthrough*>(instructions.at(current_instruction_index))) {
      auto line_num_label = find_line_num_label(is_cjump_fallthrough->label, instructions);

      if (line_num_label < instructions.size() - 1) {
        auto in_set = in[line_num_label+1];
        output.insert(output.end(), in_set.begin(), in_set.end());
      }

      // Fallthrough
      if (current_instruction_index < instructions.size() - 1) {
        auto in_set = in[current_instruction_index+1];
        output.insert(output.end(), in_set.begin(), in_set.end());
      }
    } else if (auto is_goto = dynamic_cast<Goto*>(instructions.at(current_instruction_index))) {
      auto line_num_label = find_line_num_label(is_goto->label, instructions);
      if (line_num_label < instructions.size() - 1) {
        auto in_set = in[line_num_label+1];
        output.insert(output.end(), in_set.begin(), in_set.end());
      }
    } else if (auto is_return = dynamic_cast<Instruction_ret*>(instructions.at(current_instruction_index))) {
      // Do nothing - return instructions have no out set
    } else  {
      if (current_instruction_index < instructions.size() - 1) {
        output = in[current_instruction_index+1];
      }
    }
    return output;
  }

  vector<string> get_str_vec(vector<std::reference_wrapper<Item*>> items){
    vector<string> new_vec = {};
    for (Item* i : items){
      new_vec.push_back(i->item_to_string());
    }
    return new_vec;
  }

  void get_in_out_sets(Program p, vector<vector<string>> &in, vector<vector<string>> &out, vector<vector<string>> &kill){
    auto instructions = (p.functions.front())->instructions;
    vector<vector<string>> gen(instructions.size());
    bool changed = false;

    for (int j=0; j<instructions.size(); j++) {
      auto current_i = instructions[j];
      auto temp_gen = current_i->generate_gen();
      auto temp_kill = current_i->generate_kill();
      gen[j] = get_str_vec(temp_gen);
      kill[j] = get_str_vec(temp_kill);
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

  void generate_in_out_sets(Program p){
    auto instructions = (p.functions.front())->instructions;
    vector<vector<string>> kill(instructions.size());
    vector<vector<string>> in(instructions.size());
    vector<vector<string>> out(instructions.size());

    get_in_out_sets(p, in, out, kill);
    cout << "(\n(in\n";
    for (int i=0; i<in.size(); i++) {
      cout << "(";
      L2::print_vector(in[i]);
      cout << ")\n";
    }
    cout << ")\n\n(out\n";
    for (int i=0; i<out.size(); i++) {
      cout << "(";
      L2::print_vector(out[i]);
      cout << ")\n";
    }
    cout << ")\n\n)";
    return ;
  }
}
