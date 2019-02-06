#include <utils.h>
#include <vector>
#include <iostream>
#include <string>
#include <iterator>
#include <L2.h>

using namespace std;

namespace L2{

  void print_vector(vector<string> v) {
    for (auto item: v) {
      if (item[0] != '%'){
        cout << item << " ";
      } else {
        cout << item.substr(1) << " ";
      }
    }
  }

  void print_function(Function* f){
    cout << '(' << f->name << "\n\t" << to_string(f->arguments) << " " << to_string(f->locals) << endl;

    auto instructions = f->instructions;
    for (auto i : instructions){
      if (Assignment* assignment = dynamic_cast<Assignment*>(i)){
        cout << "\t" << assignment->d->item_to_string() << " " << assignment->op << " " << assignment->s->item_to_string() << "\n";
      } else if (AssignmentCmp* ass_cmp = dynamic_cast<AssignmentCmp*>(i)) {
        cout << "\t" << ass_cmp->d->item_to_string() << " <- " << ass_cmp->c.to_string() << "\n";
      } else if (Cjump* cjump = dynamic_cast<Cjump*>(i)) {
        cout << "\tcjump " << cjump->c.to_string() << " " << cjump->label1 << " " << cjump->label2 << "\n";
      } else if (Cjump_fallthrough* cjump_fallthrough = dynamic_cast<Cjump_fallthrough*>(i)) {
        cout << "\tcjump " << cjump_fallthrough->c.to_string() << " " << cjump_fallthrough->label << "\n";
      } else if (Inc_or_dec* inc_or_dec = dynamic_cast<Inc_or_dec*>(i)) {
        cout << "\t" << inc_or_dec->w->item_to_string() << inc_or_dec->op << "\n";
      } else if (At_arithmetic* at = dynamic_cast<At_arithmetic*>(i)) {
        cout << "\t" << at->dest->item_to_string() << " @ " << at->w1->item_to_string() << " " << at->w2->item_to_string() << " " << at->n->item_to_string() << "\n";
      } else if (Label_instruction* label = dynamic_cast<Label_instruction*>(i)) {
        cout << "\t" << label->label << "\n";
      } else if (Goto* go = dynamic_cast<Goto*>(i)) {
        cout << "\tgoto " << go->label << "\n";
      } else if (Instruction_ret* ret = dynamic_cast<Instruction_ret*>(i)) {
        cout << "\treturn";
      } else if (Custom_func_call* cus_func = dynamic_cast<Custom_func_call*>(i)) {
        cout << "\tcall " << cus_func->u->item_to_string() << " " << cus_func->n->item_to_string() << "\n";
      } else if (System_func_call* sys_func = dynamic_cast<System_func_call*>(i)) {
        if (sys_func->system_func == "print"){
          cout << "\tcall print 1\n";
        } else if (sys_func->system_func == "allocate"){
          cout << "\tcall allocate 2\n";
        } else {
          cout << "\tcall array-error 2\n";
        }
      }
    }
    cout << ')';
  }
}
