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

#include <code_generator.h>
#include <utils.h>

using namespace std;

namespace L3{
  void generate_code(Function* f, vector<stack<string>> all_l2_instructions, unordered_map<string, string> label_map, ofstream &outputFile){
    auto arguments = f->arguments;
    outputFile << '(' << f->name << "\n\t" << to_string(arguments.size()) << " 0\n";
    for (int i = 0; i < arguments.size(); i++){
      if (i < 6){
        outputFile << "\t" << arguments[i]->to_string() << " <- " << L3::argument_registers[i] + "\n";
      } else {
        outputFile << "\t" << arguments[i]->to_string() << " <- stack-arg " << to_string((arguments.size()-1-i)*8) + "\n";
      }
    }

    for (stack<string> l2_instructions : all_l2_instructions){
      while(!l2_instructions.empty()){
        outputFile << l2_instructions.top();
        l2_instructions.pop();
      }
    }
    outputFile << ")\n\n";

    return ;
  }
}
