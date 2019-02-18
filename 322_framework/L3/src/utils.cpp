#include <utils.h>
#include <vector>
#include <iostream>
#include <string>
#include <iterator>
#include <stack>
#include <L3.h>
#include <algorithm>
#include <unordered_map>

using namespace std;

namespace L3{
  vector<string> argument_registers = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
  vector<string> instruction_types = {"ASSIGN", "OP", "CMP", "LOAD", "STORE", "GOTO", "LABELI", "JUMP", "RETVOID", "RET", "CALL", "CALLSTORE"};

  bool is_int(std::string s){
    if(s.empty()) return false;
    if(!isdigit(s[0]) && s[0] != '-' && s[0] != '+') return false;

    char* p;
    std::strtol(s.c_str(), &p, 10);
    return (*p == 0);
  }

  bool contains(vector<string> v, string s) {
    return find(v.begin(), v.end(), s) != v.end();
  }

  std::unordered_map<std::string, std::string> create_label_map(Program &p, Function* f){
    std::unordered_map<std::string, std::string> label_map;
    for (Label* label : f->labels){
      std::string label_name = label->name;
      if (label_map.count(label_name) == 0){
        label_map[label_name] = p.longest_label_name + std::to_string(p.label_count);
        p.label_count++;
      }
    }
    return label_map;
  }

  string instruction_type_string(Instruction_type s) {
	return instruction_types[s];
}

void print_stack(stack<string> s) {
	while (!s.empty()) {
		cout << s.top()<<endl;
		s.pop();
	}
}
}
