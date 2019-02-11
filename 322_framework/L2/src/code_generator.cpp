#include <string>
#include <iostream>
#include <fstream>

#include <code_generator.h>

using namespace std;

namespace L2{
  void generate_code(Program p){

    /*
     * Open the output file.
     */
    std::ofstream outputFile;
    outputFile.open("prog.L1");

    /*
     * Generate target code
     */
    outputFile << "(" << p.entryPointLabel << "\n";
    for (auto f : p.functions){

      outputFile << '(' << f->name << "\n\t" << to_string(f->arguments) << " " << to_string(f->locals) << endl;

      auto instructions = f->instructions;
      for (auto i : instructions){
        if (Assignment* assignment = dynamic_cast<Assignment*>(i)){
          outputFile << "\t" << assignment->d->item_to_string() << " ";
          if (assignment->op == "<- stack-arg"){
            Num_item* num = dynamic_cast<Num_item*>(assignment->s);
            outputFile << "<- mem rsp " << to_string(num->n + f->locals * 8) << "\n";
          } else {
            outputFile << assignment->op << " " << assignment->s->item_to_string() << "\n";
          }
        } else if (AssignmentCmp* ass_cmp = dynamic_cast<AssignmentCmp*>(i)) {
          outputFile << "\t" << ass_cmp->d->item_to_string() << " <- " << ass_cmp->c.to_string() << "\n";
        } else if (Cjump* cjump = dynamic_cast<Cjump*>(i)) {
          outputFile << "\tcjump " << cjump->c.to_string() << " " << cjump->label1 << " " << cjump->label2 << "\n";
        } else if (Cjump_fallthrough* cjump_fallthrough = dynamic_cast<Cjump_fallthrough*>(i)) {
          outputFile << "\tcjump " << cjump_fallthrough->c.to_string() << " " << cjump_fallthrough->label << "\n";
        } else if (Inc_or_dec* inc_or_dec = dynamic_cast<Inc_or_dec*>(i)) {
          outputFile << "\t" << inc_or_dec->w->item_to_string() << inc_or_dec->op << "\n";
        } else if (At_arithmetic* at = dynamic_cast<At_arithmetic*>(i)) {
          outputFile << "\t" << at->dest->item_to_string() << " @ " << at->w1->item_to_string() << " " << at->w2->item_to_string() << " " << to_string(at->n) << "\n";
        } else if (Label_instruction* label = dynamic_cast<Label_instruction*>(i)) {
          outputFile << "\t" << label->label << "\n";
        } else if (Goto* go = dynamic_cast<Goto*>(i)) {
          outputFile << "\tgoto " << go->label << "\n";
        } else if (Instruction_ret* ret = dynamic_cast<Instruction_ret*>(i)) {
          outputFile << "\treturn\n";
        } else if (Custom_func_call* cus_func = dynamic_cast<Custom_func_call*>(i)) {
          outputFile << "\tcall " << cus_func->u->item_to_string() << " " << to_string(cus_func->n) << "\n";
        } else if (System_func_call* sys_func = dynamic_cast<System_func_call*>(i)) {
          if (sys_func->system_func == "print"){
            outputFile << "\tcall print 1\n";
          } else if (sys_func->system_func == "allocate"){
            outputFile << "\tcall allocate 2\n";
          } else {
            outputFile << "\tcall array-error 2\n";
          }
        }
      }
      outputFile << ")\n\n";
    }
    outputFile << ")\n";

    /*
     * Close the output file.
     */
    outputFile.close();

    return ;
  }
}
