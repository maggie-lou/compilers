#pragma once

#include <L3.h>
#include <vector>
#include <string>
#include <iterator>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
#include <iostream>
#include <unordered_map>
#include <stack>

using namespace std;

namespace L3{

  void generate_code(Function* f, vector<stack<string>> all_l2_instructions, unordered_map<string, string> label_map, ofstream &outputFile);

}
