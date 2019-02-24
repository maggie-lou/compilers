#include <string>
#include <iostream>
#include <fstream>

#include <code_generator.h>

using namespace std;

namespace IR{

  int64_t get_offset(){

  }

  void generate_code(Program p){

    /*
     * Open the output file.
     */
    std::ofstream outputFile;
    outputFile.open("prog.L3");

    /*
     * Generate target code
     */
    for (auto f : p.functions){

      outputFile << 'define ' << f->name << "(" << f->arg_to_string() << "){\n";

      auto instructions = f->instructions;
      for (auto i : instructions){
        if (Instruction_assign* assign = dynamic_cast<Instruction_assign*>(i)){
          outputFile << "\t" << assign->dest->to_string() << " <- " << assign->source->to_string() << "\n";
        } else if (Instruction_op* op = dynamic_cast<Instruction_op*>(i)) {
          outputFile << "\t" << assign->dest->to_string() << " <- " << assign->t1->to_string() << " " << op << " " << assign->t2->to_string() << "\n";
        } else if (Instruction_load* load = dynamic_cast<Instruction_load*>(i)) {

          outputFile

        } else if (Instruction_store* store = dynamic_cast<Instruction_store*>(i)) {

        } else if (Instruction_length* length_i = dynamic_cast<Instruction_length*>(i)) {
          string v0 = p.longest_var + "_" + to_string(p.var_count);
          p.var_count++;
          string v1 = p.longest_var + "_" + to_string(p.var_count);
          p.var_count++;
          string v2 = p.longest_var + "_" + to_string(p.var_count);
          p.var_count++;
          outputFile << "\t" << v0 << " <- " << to_string(length_i->dimension) << " * 8\n";
          outputFile << "\t" << v1 << " <- " << v0 << " + 16\n";
          outputFile << "\t" << v2 << " <- " << length_i->source->to_string() << " + " << v1 << "\n";
          outputFile << "\t" << length_i->dest->to_string() << " <- load " << v2 << "\n";

        } else if (Instruction_call* call = dynamic_cast<Instruction_call*>(i)) {

        } else if (Instruction_call_store* call_store = dynamic_cast<Instruction_call_store*>(i)) {

        } else if (Instruction_array* new_array = dynamic_cast<Instruction_array*>(i)) {
          int64_t size = new_array->args->size();
          string dest = new_array->dest->to_string();
          string v0 = p.longest_var + "_" + to_string(p.var_count);
          p.var_count++;
          for (Item* arg : new_array->args){
            string temp = p.longest_var + "_" + to_string(p.var_count);
            p.var_count++;
            outputFile << "\t" << temp << " <- " << arg->to_string() <<  " >> 1\n";
            outputFile << "\t" << v0 << " <- " << v0 <<  " * " << temp << "\n";
          }

          outputFile << "\t" << v0 << " <- " << v0 <<  " + " << to_string(size+1) << "\n";
          outputFile << "\t" << v0 << " <- " << v0 <<  " << 1\n";
          outputFile << "\t" << v0 << " <- " << v0 <<  " + 1\n";

          outputFile << "\t" << dest << " <- call allocate(" << v0 << ", 1)\n";

          string v1 = p.longest_var + "_" + to_string(p.var_count);
          p.var_count++;
          outputFile << "\t" << v1 << " <- " << dest + " + 8\n";
          outputFile << "\tstore " << v1 << " <- " << to_string((size << 1) + 1) << "\n";

          for (int64_t i  = 0; i < size; i++){
            string temp = p.longest_var + "_" + to_string(p.var_count);
            p.var_count++;
            outputFile << "\t" << temp << " <- " << dest + " + " << to_string(16+8*i) << "\n";
            outputFile << "\tstore " << temp << " <- " << new_array->args[i] << "\n";
          }

        } else if (Instruction_goto* goto = dynamic_cast<Instruction_goto*>(i)) {

        } else if (Instruction_label* label_i = dynamic_cast<Instruction_label*>(i)) {

        } else if (Instruction_jump* jump = dynamic_cast<Instruction_jump*>(i)) {

        } else if (Instruction_ret_void* ret_void = dynamic_cast<Instruction_ret_void*>(i)) {

        } else if (Instruction_ret* ret = dynamic_cast<Instruction_ret*>(i)) {

        }
      }
      outputFile << "}\n\n";
    }


    /*
     * Close the output file.
     */
    outputFile.close();

    return ;
  }
}
