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


  void generate_code_naive(Program p){
    ofstream outputFile;
    outputFile.open("prog.L2");
    outputFile << "(:main\n";

    for (auto f : p.functions){
      auto label_map = L3::create_label_map(p, f);
      auto arguments = f->arguments;
      outputFile << '(' << f->name << "\n\t" << to_string(arguments.size()) << " 0\n";
      for (int i = 0; i < arguments.size(); i++){
        if (i < 6){
          outputFile << "\t" << arguments[i]->to_string() << " <- " << L3::argument_registers[i] + "\n";
        } else {
          outputFile << "\t" << arguments[i]->to_string() << " <- stack-arg " << to_string((arguments.size()-1-i)*8) + "\n";
        }
      }

      for (Instruction* i : f->instructions) {

        if (Instruction_assign* assign = dynamic_cast<Instruction_assign*>(i)) {
          string deststr = assign->dest->to_string();
          string sourcestr = assign->source->to_string();
          if (deststr != sourcestr){
            outputFile << "\t" + deststr + " <- " + sourcestr + "\n";
          }


        } else if (Instruction_op* op = dynamic_cast<Instruction_op*>(i)) {
          if (auto dest1 = dynamic_cast<Variable*>(op->dest)){
            if (auto v2 = dynamic_cast<Variable*>(op->t2)){
              if (dest1->name == v2->name){
                Item* temp = op->t1;
                op->t1 = op->t2;
                op->t2 = temp;
              }
            }
          }
          string deststr = op->dest->to_string();
          string t1str = op->t1->to_string();
          if(deststr != t1str){
            outputFile << "\t" << deststr << " <- " << t1str << "\n";
          }
        
          outputFile << "\t" << deststr << " " << op->op << "= " << op->t2->to_string() << "\n";

        } else if (Instruction_cmp* cmp = dynamic_cast<Instruction_cmp*>(i)) {
          bool flip = false;
          if (cmp->cmp == ">"){
            cmp->cmp = "<";
            flip = true;
          } else if (cmp->cmp == ">="){
            cmp->cmp = "<=";
            flip = true;
          }
          if (flip){
            outputFile << "\t" << cmp->dest->to_string() << " <- " << cmp->t2->to_string() << " " << cmp->cmp<<" "<<cmp->t1->to_string()<<"\n";
          } else {
            outputFile << "\t" << cmp->dest->to_string() << " <- " << cmp->t1->to_string() << " " << cmp->cmp<<" "<<cmp->t2->to_string()<<"\n";
          }

        } else if (Instruction_load* load = dynamic_cast<Instruction_load*>(i)) {
          outputFile << "\t" << load->dest->to_string()+" <- mem "+load->source->to_string()+"  0\n";
        } else if (Instruction_store* store = dynamic_cast<Instruction_store*>(i)) {
          outputFile << "\tmem "<<store->dest->to_string()<<" 0 <- "<<store->source->to_string()<<"\n";
        } else if (Instruction_goto* goto_i = dynamic_cast<Instruction_goto*>(i)) {
          outputFile << "\tgoto " << label_map[goto_i->label->to_string()] << "\n";

        } else if (Instruction_call* call = dynamic_cast<Instruction_call*>(i)) {
          for (int i=0; i<call->args.size(); i++) {
            Item* child = call->args[i];
            if (i >= argument_registers.size()) {
              int stack_loc = -16 - 8 * (i - argument_registers.size());
              outputFile << "\tmem rsp " << to_string(stack_loc) << " <- " << child->to_string() << "\n";
            } else {
              string arg_register = argument_registers[i];
              outputFile << "\t" << argument_registers[i] << " <- " << child->to_string() << "\n";
            }
          }

          if (call->callee->type != L3::Item_type::SYSCALL){
            outputFile << "\tmem rsp -8 <- " << p.longest_label_name << std::to_string(p.label_count) << "_" << call->callee->to_string().substr(1)+"\n";

          }
          outputFile << "\tcall " << call->callee->to_string() << " " << to_string(call->args.size()) << "\n";
          if (call->callee->type != L3::Item_type::SYSCALL){
            outputFile << "\t"<< p.longest_label_name<<std::to_string(p.label_count)<<"_"<<call->callee->to_string().substr(1)<<"\n";
          }
          p.label_count++;


        } else if (Instruction_call_store* call_store = dynamic_cast<Instruction_call_store*>(i)) {
          for (int i=0; i<call_store->args.size(); i++) {
            Item* child = call_store->args[i];
            if (i >= argument_registers.size()) {
              int stack_loc = -16 - 8 * (i - argument_registers.size());
              outputFile << "\tmem rsp " << to_string(stack_loc) << " <- " << child->to_string() << "\n";
            } else {
              string arg_register = argument_registers[i];
              outputFile << "\t" << argument_registers[i] << " <- " << child->to_string() << "\n";
            }
          }

          if (call_store->callee->type != L3::Item_type::SYSCALL){
            outputFile << "\tmem rsp -8 <- " << p.longest_label_name << std::to_string(p.label_count) << "_" << call_store->callee->to_string().substr(1)+"\n";

          }
          outputFile << "\tcall " << call_store->callee->to_string() << " " << to_string(call_store->args.size()) << "\n";
          if (call_store->callee->type != L3::Item_type::SYSCALL){
            outputFile << "\t"<< p.longest_label_name<<std::to_string(p.label_count)<<"_"<<call_store->callee->to_string().substr(1)<<"\n";
          }
          outputFile << "\t" <<call_store->dest->to_string() + " <- rax\n";
          p.label_count++;

        } else if (Instruction_label* label_i = dynamic_cast<Instruction_label*>(i)) {
          outputFile << "\t" << label_map[label_i->label->to_string()] << "\n";

        } else if (Instruction_jump* jump_i = dynamic_cast<Instruction_jump*>(i)) {
          outputFile << "\tcjump" << jump_i->var->to_string() << " = 1 " << label_map[jump_i->label->to_string()] << "\n";

        } else if (Instruction_ret_void* ret_void = dynamic_cast<Instruction_ret_void*>(i)) {
          outputFile << "\treturn\n";

        } else if (Instruction_ret* ret = dynamic_cast<Instruction_ret*>(i)) {
          outputFile << "\trax <- " << ret->t->to_string()<<"\n";
          outputFile << "\treturn\n";

        }
      }

      outputFile << ")\n\n";
    }

    outputFile << ")\n";
    outputFile.close();

    return ;
  }

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
