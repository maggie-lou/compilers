#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>

#include <code_generator.h>
#include <utils.h>

using namespace std;

namespace LB {

  string to_indices_string(vector<string> indices) {
    string s = "";
    for (string i : indices) {
      s = s + "[" + i + "]";
    }
    return s;
  }

  string to_args_string(vector<Item*> args) {
    string s = "(";
    for (Item* i : args) {
      s = s + i->to_string() + ",";
    }
    if (!args.empty()){
      s = s.substr(0, s.length() - 1);
    }

    s = s + ")";
    return s;
  }

  string get_function_args_string(Function* f) {
    string arg_string = "";
    for (Variable* arg : f->arguments) {
      arg_string = arg_string + arg->type.to_string() + " " + arg->name + ", ";
    }
    arg_string = arg_string.substr(0, arg_string.length()-2); // remove trailing comma
    return arg_string;
  }

  void generate_code(Program p){
    /*
     * Open the output file.
     */
    ofstream outputFile;
    outputFile.open("prog.a");

    /*
     * Generate target code
     */

    for (auto f : p.functions){

      // outputFile << "define " << f->type.to_string() << " :" << f->name << "(" << get_function_args_string(f) << "){" << endl;

      auto instructions = f->instructions;

      for (Instruction* i : instructions) {

        if (Instruction_definition* def = dynamic_cast<Instruction_definition*>(i)) {
          for (Variable* var : def->vars){
            outputFile << "\t" << def->type.to_string() << " " << var->to_string() << endl;
          }

        } else if (Instruction_assign* assign = dynamic_cast<Instruction_assign*>(i)){
          outputFile << "\t" << assign->dest->to_string() << " <- " << assign->source->to_string() << "\n";

        } else if (Instruction_op* op = dynamic_cast<Instruction_op*>(i)) {

        } else if (Instruction_if* if_i = dynamic_cast<Instruction_if*>(i)) {

        } else if (Instruction_while* while_i = dynamic_cast<Instruction_while*>(i)) {

        } else if (Instruction_continue* continue_i = dynamic_cast<Instruction_continue*>(i)) {

        } else if (Instruction_break* break_i = dynamic_cast<Instruction_break*>(i)) {

        } else if (Instruction_load* load = dynamic_cast<Instruction_load*>(i)) {

        } else if (Instruction_store* store = dynamic_cast<Instruction_store*>(i)) {

        } else if (Instruction_length* length_i = dynamic_cast<Instruction_length*>(i)) {

        } else if (Instruction_call* call = dynamic_cast<Instruction_call*>(i)) {

        } else if (Instruction_call_store* call_store = dynamic_cast<Instruction_call_store*>(i)) {

        } else if (Instruction_print* print = dynamic_cast<Instruction_print*>(i)) {

        } else if (Instruction_array* new_array = dynamic_cast<Instruction_array*>(i)) {

        } else if (Instruction_label* label_i = dynamic_cast<Instruction_label*>(i)) {

        } else if (Instruction_ret_void* ret_void = dynamic_cast<Instruction_ret_void*>(i)) {

        } else if (Instruction_ret* ret = dynamic_cast<Instruction_ret*>(i)) {

        }
      }
      outputFile << "}\n\n";
    }
  }

}
