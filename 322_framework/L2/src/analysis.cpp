#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>
#include <iterator>

#include <analysis.h>

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
        if (label == label_instruction-> label) {
          return i;
        }
      }
    }
    return -1;
  }

  vector<string> generate_out_set(int current_instruction_index, vector<Instruction*> instructions, vector<vector<string>> in) {
    vector<string> output;

    Cjump* is_cjump = dynamic_cast<Cjump*>(instructions.at(current_instruction_index));
    if (is_cjump) {
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
    } else {
      Cjump_fallthrough* is_cjump_fallthrough = dynamic_cast<Cjump_fallthrough*>(instructions.at(current_instruction_index));
      if (is_cjump_fallthrough) {
        auto line_num_label = find_line_num_label(is_cjump_fallthrough->label, instructions);

        if (line_num_label < instructions.size() - 1) {
          auto in_set = in[line_num_label+1];
          output.insert(output.end(), in_set.begin(), in_set.end());
        }
      } else {
        if (current_instruction_index < instructions.size() - 1) {
          output = in[current_instruction_index+1];
        }
      }
    }
    return output;
  }

  void print_vector(vector<string> v) {
    for (auto item: v) {
      cout << item << " ";
    }
  }

  void generate_in_out_sets(Program p){
    auto instructions = (p.functions.front())->instructions;
    vector<vector<string>> gen(instructions.size());
    vector<vector<string>> kill(instructions.size());
    vector<vector<string>> in(instructions.size());
    vector<vector<string>> out(instructions.size());
    bool changed = false;

    for (int j=0; j<instructions.size(); j++) {
      auto current_i = instructions[j];
      gen[j] = current_i->generate_gen();
      kill[j] = current_i->generate_kill();
      in[j] = {};
      out[j] = {};
    }
    // cout << "gen\n";
    // for (int i = 0; i < gen.size(); i++)
    //   {
    //       for (int j = 0; j < gen[i].size(); j++)
    //       {
    //           cout << gen[i][j] << " ";
    //       }
    //       cout << "\n";
    //   }
    // cout << "kill\n";
    // for (int i = 0; i < kill.size(); i++)
    //   {
    //       for (int j = 0; j < kill[i].size(); j++)
    //       {
    //           cout << kill[i][j] << " ";
    //       }
    //       cout << "\n";
    //   }

    do {
      changed = false;

      for (int j=instructions.size()-1; j>=0; j--) {
        // cout << "Instruction: " << j << endl;
        // cout << "Gen: ";
        // print_vector(gen[j]);
        // cout << endl;
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

    cout << "(\n(in\n";
    for (int i=0; i<instructions.size(); i++) {
      cout << "(";
      print_vector(in[i]);
      cout << ")\n";
    }
    cout << ")\n\n(out\n";
    for (int i=0; i<instructions.size(); i++) {
      cout << "(";
      print_vector(out[i]);
      cout << ")\n";
    }
    cout << ")\n\n)";

    return ;
  }
}
