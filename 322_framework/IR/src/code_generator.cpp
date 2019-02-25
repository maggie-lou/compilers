#include <string>
#include <iostream>
#include <fstream>

#include <code_generator.h>

using namespace std;

namespace IR{

  string get_offset(Program &p, int64_t size, string arr_name, ofstream &outputFile, vector<Item*> indices){

    string addr = p.longest_var + "_" + to_string(p.var_count);
    // p.var_count++;
    // string product = p.longest_var + "_" + to_string(p.var_count);
    // p.var_count++;
    //
    // outputFile << "\t" << addr << " <- 0\n";
    //
    // // get the size of each dimension
    // for (int64_t i = 0; i < size-1; i++){
    //   string temp_addr = p.longest_var + "_" + to_string(p.var_count);
    //   p.var_count++;
    //   string temp_var = p.longest_var + "_" + to_string(p.var_count);
    //   p.var_count++;
    //   string temp_sum = p.longest_var + "_" + to_string(p.var_count);
    //   p.var_count++;
    //   outputFile << "\t" << temp_addr << " <- " << arr_name << " + " << to_string(24+i*8) << "\n";
    //   outputFile << "\t" << temp_var << " <- load " << temp_addr << "\n";
    //   outputFile << "\t" << temp_var << " <- " << temp_var << " >> 1\n";
    //   outputFile << "\t" << product << " <- " << product << " * " << temp_var << "\n";
    //   outputFile << "\t" << temp_sum << " <- " << product << " * " << indices[size-i-2]->to_string() << "\n";
    //   outputFile << "\t" << addr << " <- " << addr << " + " << temp_sum << "\n";
    // }
    // outputFile << "\t" << addr << " <- " << addr << " + " << indices[size-1]->to_string() << "\n";
    // outputFile << "\t" << addr << " <- " << addr << " * 8\n";
    // outputFile << "\t" << addr << " <- " << addr << " + 16\n";
    // string temp = p.longest_var + "_" + to_string(p.var_count);
    // p.var_count++;
    // outputFile << "\t" << temp << " <- " << size << " * 8\n";
    // outputFile << "\t" << addr << " <- " << addr << " + " << size << "\n";
    // outputFile << "\t" << addr << " <- " << addr << " + " << arr_name << "\n";
    //
    return addr;
  }

  void generate_code(Program p){

    /*
     * Open the output file.
     */
    // ofstream outputFile;
    // outputFile.open("prog.L3");
    //
    // #<{(|
    //  * Generate target code
    //  |)}>#
    // for (auto f : p.functions){
    //
    //   outputFile << 'define ' << f->name << "(" << f->arg_to_string() << "){\n";
    //
    //   auto instructions = f->instructions;
    //   for (auto i : instructions){
    //     if (Instruction_assign* assign = dynamic_cast<Instruction_assign*>(i)){
    //       outputFile << "\t" << assign->dest->to_string() << " <- " << assign->source->to_string() << "\n";
    //
    //     } else if (Instruction_op* op = dynamic_cast<Instruction_op*>(i)) {
    //       outputFile << "\t" << assign->dest->to_string() << " <- " << assign->t1->to_string() << " " << op << " " << assign->t2->to_string() << "\n";
    //
    //     } else if (Instruction_load* load = dynamic_cast<Instruction_load*>(i)) {
    //       string source = load->source->to_string();
    //       int64_t size = load->indices->size();
    //       string addr = get_offset(p, size, source, outputFile, load->indices);
    //       outputFile << "\t" << load->dest->to_string() << " <- load " << addr << "\n";
    //
    //     } else if (Instruction_store* store = dynamic_cast<Instruction_store*>(i)) {
    //       string dest = store->dest->to_string();
    //       int64_t size = store->indices->size();
    //       string addr = get_offset(p, size, dest, outputFile, store->indices);
    //       outputFile << "\tstore " << addr << " <- " << store->source->to_string() << "\n";
    //
    //     } else if (Instruction_length* length_i = dynamic_cast<Instruction_length*>(i)) {
    //       string v0 = p.longest_var + "_" + to_string(p.var_count);
    //       p.var_count++;
    //       string v1 = p.longest_var + "_" + to_string(p.var_count);
    //       p.var_count++;
    //       string v2 = p.longest_var + "_" + to_string(p.var_count);
    //       p.var_count++;
    //       outputFile << "\t" << v0 << " <- " << to_string(length_i->dimension) << " * 8\n";
    //       outputFile << "\t" << v1 << " <- " << v0 << " + 16\n";
    //       outputFile << "\t" << v2 << " <- " << length_i->source->to_string() << " + " << v1 << "\n";
    //       outputFile << "\t" << length_i->dest->to_string() << " <- load " << v2 << "\n";
    //
    //     } else if (Instruction_call* call = dynamic_cast<Instruction_call*>(i)) {
    //       outputFile << "\tcall " << call->callee->to_string() << "(" << call->arg_to_string() << ")\n";
    //
    //     } else if (Instruction_call_store* call_store = dynamic_cast<Instruction_call_store*>(i)) {
    //       outputFile << "\t" << call_store->dest->name << " <- call " << call->callee->to_string() << "(" << call->arg_to_string() << ")\n";
    //
    //     } else if (Instruction_array* new_array = dynamic_cast<Instruction_array*>(i)) {
    //       int64_t size = new_array->args->size();
    //       string dest = new_array->dest->to_string();
    //       string v0 = p.longest_var + "_" + to_string(p.var_count);
    //       p.var_count++;
    //       for (Item* arg : new_array->args){
    //         string temp = p.longest_var + "_" + to_string(p.var_count);
    //         p.var_count++;
    //         outputFile << "\t" << temp << " <- " << arg->to_string() <<  " >> 1\n";
    //         outputFile << "\t" << v0 << " <- " << v0 <<  " * " << temp << "\n";
    //       }
    //
    //       outputFile << "\t" << v0 << " <- " << v0 <<  " + " << to_string(size+1) << "\n";
    //       outputFile << "\t" << v0 << " <- " << v0 <<  " << 1\n";
    //       outputFile << "\t" << v0 << " <- " << v0 <<  " + 1\n";
    //
    //       outputFile << "\t" << dest << " <- call allocate(" << v0 << ", 1)\n";
    //
    //       string v1 = p.longest_var + "_" + to_string(p.var_count);
    //       p.var_count++;
    //       outputFile << "\t" << v1 << " <- " << dest + " + 8\n";
    //       outputFile << "\tstore " << v1 << " <- " << to_string((size << 1) + 1) << "\n";
    //
    //       for (int64_t i  = 0; i < size; i++){
    //         string temp = p.longest_var + "_" + to_string(p.var_count);
    //         p.var_count++;
    //         outputFile << "\t" << temp << " <- " << dest + " + " << to_string(16+8*i) << "\n";
    //         outputFile << "\tstore " << temp << " <- " << new_array->args[i] << "\n";
    //       }
    //
    //     } else if (Instruction_goto* goto_i = dynamic_cast<Instruction_goto*>(i)) {
    //       outputFile << "\tbr " << goto_i->label->name << "\n";
    //
    //     } else if (Instruction_label* label_i = dynamic_cast<Instruction_label*>(i)) {
    //       outputFile << "\t" << label_i->label->name << "\n";
    //
    //     } else if (Instruction_jump* jump = dynamic_cast<Instruction_jump*>(i)) {
    //       outputFile << "\tbr " << jump->check->to_string() << " " << jump->label1->name << "\n";
    //       outputFile << "\tbr " << jump->label2->name << "\n";
    //
    //     } else if (Instruction_ret_void* ret_void = dynamic_cast<Instruction_ret_void*>(i)) {
    //       outputFile << "\treturn\n";
    //
    //     } else if (Instruction_ret* ret = dynamic_cast<Instruction_ret*>(i)) {
    //       outputFile << "\treturn " << ret->t->to_string() << "\n";
    //     }
    //   }
    //   outputFile << "}\n\n";
    // }
    //
    //
    // #<{(|
    //  * Close the output file.
    //  |)}>#
    // outputFile.close();

    return ;
  }
}
