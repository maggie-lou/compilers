#include <utils.h>
#include <vector>
#include <iostream>
#include <string>
#include <iterator>
#include <L3.h>

namespace L3{
  bool is_int(std::string s){
    if(s.empty()) return false;
    if(!isdigit(s[0]) && s[0] != '-' && s[0] != '+') return false;

    char* p;
    std::strtol(s.c_str(), &p, 10);
    return (*p == 0);
  }

  bool contains(vector<string> v, string s) {
    return find(begin(v), end(v), s) != end(v);
  }

  string get_var_name(Item* i) {
    if (auto var = dynamic_cast<Variable*>(i)) {
      return var->name;
    }
  }
}
