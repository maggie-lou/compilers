#pragma once

#include <L3.h>
#include <unordered_map>
#include <stack>

using namespace std;

namespace L3{

  extern vector<string> argument_registers;

  bool is_int(std::string s);
  bool contains(vector<string> v, string s);
  string instruction_type_string(Instruction_type s);
  void print_stack(stack<string> s);
  unordered_map<string, string> create_label_map(Program &p, Function* f);
  void print_tree(Node* root, int64_t layer);
}
