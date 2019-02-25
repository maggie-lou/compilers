#include <string>
#include <iostream>
#include <fstream>

#include <code_generator.h>

using namespace std;

namespace IR{

  string get_offset(Program &p, string arr_name, ofstream &outputFile, vector<Item*> indices){
    // cout << "in get offset\n";
    int64_t size = indices.size();
    // cout << "size: " << to_string(size) << "\n";
    string addr = p.longest_var + "_" + to_string(p.var_count);
    p.var_count++;
    string product = p.longest_var + "_" + to_string(p.var_count);
    p.var_count++;

    outputFile << "\t" << addr << " <- 0\n";
    outputFile << "\t" << product << " <- 1\n";

    // get the size of each dimension
    for (int64_t i = 0; i < size-1; i++){
      // cout << "i: " << to_string(i) << "\n";
      // cout << "size-i-2: " << to_string(size-2-i) << "\n";
      string temp_addr = p.longest_var + "_" + to_string(p.var_count);
      p.var_count++;
      string temp_var = p.longest_var + "_" + to_string(p.var_count);
      p.var_count++;
      string temp_sum = p.longest_var + "_" + to_string(p.var_count);
      p.var_count++;
      outputFile << "\t" << temp_addr << " <- " << arr_name << " + " << to_string(24+i*8) << "\n";
      outputFile << "\t" << temp_var << " <- load " << temp_addr << "\n";
      outputFile << "\t" << temp_var << " <- " << temp_var << " >> 1\n";
      outputFile << "\t" << product << " <- " << product << " * " << temp_var << "\n";
      outputFile << "\t" << temp_sum << " <- " << product << " * " << indices[size-i-2]->to_string() << "\n";
      outputFile << "\t" << addr << " <- " << addr << " + " << temp_sum << "\n";
    }
    outputFile << "\t" << addr << " <- " << addr << " + " << indices[size-1]->to_string() << "\n";
    outputFile << "\t" << addr << " <- " << addr << " * 8\n";
    outputFile << "\t" << addr << " <- " << addr << " + 16\n";
    string temp = p.longest_var + "_" + to_string(p.var_count);
    p.var_count++;
    outputFile << "\t" << temp << " <- " << size << " * 8\n";
    outputFile << "\t" << addr << " <- " << addr << " + " << temp << "\n";
    outputFile << "\t" << addr << " <- " << addr << " + " << arr_name << "\n";

    return addr;
  }

  void generate_code(Program p){

    /*
     * Open the output file.
     */
    ofstream outputFile;
    outputFile.open("prog.L3");
    // cout << "generating code\n";

    /*
     * Generate target code
     */
    for (auto f : p.functions){
      // cout << "function name: "  << f->name << "\n";

      outputFile << "define " << f->name << "(" << f->arg_to_string() << "){\n";

      auto instructions = f->instructions;
      for (auto i : instructions){
        if (Instruction_assign* assign = dynamic_cast<Instruction_assign*>(i)){
          outputFile << "\t" << assign->dest->to_string() << " <- " << assign->source->to_string() << "\n";

        } else if (Instruction_op* op = dynamic_cast<Instruction_op*>(i)) {
          outputFile << "\t" << op->dest->to_string() << " <- " << op->t1->to_string() << " " << op->op << " " << op->t2->to_string() << "\n";

        } else if (Instruction_load* load = dynamic_cast<Instruction_load*>(i)) {
          // cout << "generating code for load instruction\n";
          string source = load->source->to_string();
          string addr = get_offset(p, source, outputFile, load->indices);
          outputFile << "\t" << load->dest->to_string() << " <- load " << addr << "\n";

        } else if (Instruction_store* store = dynamic_cast<Instruction_store*>(i)) {
          string dest = store->dest->to_string();
          string addr = get_offset(p, dest, outputFile, store->indices);
          outputFile << "\tstore " << addr << " <- " << store->source->to_string() << "\n";

        } else if (Instruction_length* length_i = dynamic_cast<Instruction_length*>(i)) {
          string v0 = p.longest_var + "_" + to_string(p.var_count);
          p.var_count++;
          string v1 = p.longest_var + "_" + to_string(p.var_count);
          p.var_count++;
          string v2 = p.longest_var + "_" + to_string(p.var_count);
          p.var_count++;
          outputFile << "\t" << v0 << " <- " << length_i->dimension->to_string() << " * 8\n";
          outputFile << "\t" << v1 << " <- " << v0 << " + 16\n";
          outputFile << "\t" << v2 << " <- " << length_i->source->to_string() << " + " << v1 << "\n";
          outputFile << "\t" << length_i->dest->to_string() << " <- load " << v2 << "\n";

        } else if (Instruction_call* call = dynamic_cast<Instruction_call*>(i)) {
          // cout << "generating code for call instruction\n";
          outputFile << "\tcall " << call->callee->to_string() << "(" << call->arg_to_string() << ")\n";

        } else if (Instruction_call_store* call_store = dynamic_cast<Instruction_call_store*>(i)) {
          // cout << "generating code for call store instruction\n";
          outputFile << "\t" << call_store->dest->name << " <- call " << call_store->callee->to_string() << "(" << call_store->arg_to_string() << ")\n";

        } else if (Instruction_array* new_array = dynamic_cast<Instruction_array*>(i)) {
          // cout << "generating code for creating array\n";
          int64_t size = new_array->args.size();
          string dest = new_array->dest->to_string();
          string v0 = p.longest_var + "_" + to_string(p.var_count);
          p.var_count++;
          outputFile << "\t" << v0 << " <- 1\n";
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
            outputFile << "\tstore " << temp << " <- " << new_array->args[i]->to_string() << "\n";
          }

        } else if (Instruction_goto* goto_i = dynamic_cast<Instruction_goto*>(i)) {
          outputFile << "\tbr " << goto_i->label->name << "\n";

        } else if (Instruction_label* label_i = dynamic_cast<Instruction_label*>(i)) {
          outputFile << "\t" << label_i->label->name << "\n";

        } else if (Instruction_jump* jump = dynamic_cast<Instruction_jump*>(i)) {
          outputFile << "\tbr " << jump->check->to_string() << " " << jump->label1->name << "\n";
          outputFile << "\tbr " << jump->label2->name << "\n";

        } else if (Instruction_ret_void* ret_void = dynamic_cast<Instruction_ret_void*>(i)) {
          outputFile << "\treturn\n";

        } else if (Instruction_ret* ret = dynamic_cast<Instruction_ret*>(i)) {
          outputFile << "\treturn " << ret->t->to_string() << "\n";
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
