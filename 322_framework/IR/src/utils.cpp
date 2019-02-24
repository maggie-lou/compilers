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
}
