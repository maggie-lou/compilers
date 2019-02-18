#include <L3.h>
#include <context_identification.h>

#include <vector>
#include <iostream>
#include <string>
#include <iterator>

using namespace std;

namespace L3{
  vector<vector<Instruction*>> generate_contexts(Function* f) {
    cout << "Generate context with #instructions: " << f->instructions.size()<<endl;
    vector<vector<Instruction*>> contexts;
    vector<Instruction*> current_context;
    for (Instruction* i : f->instructions) {
      if (!current_context.empty() && i->type == Instruction_type::LABELI) {
        contexts.push_back(current_context);
        current_context.clear();
      }
      current_context.push_back(i);
      if (i->type == Instruction_type::JUMP) {
        contexts.push_back(current_context);
        current_context.clear();
      }
    }
    if (!current_context.empty()) {
      contexts.push_back(current_context);
    }
    return contexts;
  }
}
