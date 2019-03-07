#include <utils.h>
#include <vector>
#include <iostream>
#include <string>
#include <iterator>
#include <stack>
#include <LB.h>
#include <algorithm>
#include <unordered_map>
#include <set>

using namespace std;

namespace LB{

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



  string generate_unique_label_name(Program &p) {
    string name = p.longest_label + "_" + to_string(p.label_count);
    p.label_count++;
    return name;
  }
}
