#include <utils.h>
#include <vector>
#include <iostream>
#include <string>
#include <iterator>
#include <stack>
#include <IR.h>
#include <algorithm>
#include <unordered_map>
#include <set>

using namespace std;

namespace IR{

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

  IR::Variable_type str_to_variable_type(string s) {
    IR::Variable_type var_type;
    if (s == "int64") {
      var_type = IR::Variable_type::INT64;
    } else if (s == "code") {
      var_type = IR::Variable_type::CODE;
    }else if (s == "tuple" || s.find("int64[]") != std::string::npos) {
      var_type = IR::Variable_type::ARRAY;
    } else if (s == "void") {
      var_type = IR::Variable_type::VOID;
    } else {
      var_type = IR::Variable_type::INVALID;
    }
    return var_type;
  }
}
