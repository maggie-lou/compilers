#include <string>
#include <iostream>
#include <fstream>

#include <code_generator.h>

using namespace std;

namespace IR{

  string generate_unique_var_name(Program &p) {
    string name = p.longest_var + "_" + to_string(p.var_count);
    p.var_count++;
    return name;
  }

  string get_offset(Program &p, string arr_name, ofstream &outputFile, vector<Item*> indices){
    int64_t size = indices.size();
    string addr = generate_unique_var_name(p);
    string product = generate_unique_var_name(p);

    outputFile << "\t" << addr << " <- 0\n";
    outputFile << "\t" << product << " <- 1\n";

    // get the size of each dimension
    for (int64_t i = 0; i < size-1; i++){
      string temp_addr = generate_unique_var_name(p);
      string temp_var = generate_unique_var_name(p);
      string temp_sum = generate_unique_var_name(p);
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
    string temp = generate_unique_var_name(p);
    outputFile << "\t" << temp << " <- " << size << " * 8\n";
    outputFile << "\t" << addr << " <- " << addr << " + " << temp << "\n";
    outputFile << "\t" << addr << " <- " << addr << " + " << arr_name << "\n";

    return addr;
  }

  int encode(int n) {
    return (n << 1) + 1;
  }

  void generate_code(Program p){

    /*
     * Open the output file.
     */
    ofstream outputFile;
    outputFile.open("prog.L3");

    /*
     * Generate target code
     */
    for (auto f : p.functions){
      outputFile << "define " << f->name << "(" << f->arg_to_string() << "){\n";

      auto instructions = f->instructions;
      for (auto i : instructions){
        if (Instruction_assign* assign = dynamic_cast<Instruction_assign*>(i)){
          outputFile << "\t" << assign->dest->to_string() << " <- " << assign->source->to_string() << "\n";

        } else if (Instruction_op* op = dynamic_cast<Instruction_op*>(i)) {
          outputFile << "\t" << op->dest->to_string() << " <- " << op->t1->to_string() << " " << op->op << " " << op->t2->to_string() << "\n";

        } else if (Instruction_load* load = dynamic_cast<Instruction_load*>(i)) {
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
          outputFile << "\tcall " << call->callee->to_string() << "(" << call->arg_to_string() << ")\n";

        } else if (Instruction_call_store* call_store = dynamic_cast<Instruction_call_store*>(i)) {
          outputFile << "\t" << call_store->dest->name << " <- call " << call_store->callee->to_string() << "(" << call_store->arg_to_string() << ")\n";

        } else if (Instruction_array* new_array = dynamic_cast<Instruction_array*>(i)) {
          int64_t num_dimensions = new_array->dimensions.size();
          string arr_name = new_array->dest->to_string();

          // Generate code to calculate linearized length of array
          string linearized_len_var = generate_unique_var_name(p);
          outputFile << "\t" << linearized_len_var << " <- 1\n"; // Initialize var to 1 for accumulating product
          for (Item* dim : new_array->dimensions){
            string temp_dimension_var = generate_unique_var_name(p);

            // Decode dimension
            outputFile << "\t" << temp_dimension_var << " <- " << dim->to_string() <<  " >> 1\n";
            // Generate total dimension product for array size
            outputFile << "\t" << linearized_len_var << " <- " << linearized_len_var <<  " * " << temp_dimension_var << "\n";
          }

          // Add memory for each dimension size
          if (!new_array->is_tuple) {
            outputFile << "\t" << linearized_len_var << " <- " << linearized_len_var <<  " + " << to_string(num_dimensions+1) << "\n";
          }

          // Encode
          outputFile << "\t" << linearized_len_var << " <- " << linearized_len_var <<  " << 1\n";
          outputFile << "\t" << linearized_len_var << " <- " << linearized_len_var <<  " + 1\n";

          outputFile << "\t" << arr_name << " <- call allocate(" << linearized_len_var << ", 1)\n";

          if (!new_array->is_tuple) {
            // Store number dimensions in array
            string num_dimensions_var = generate_unique_var_name(p);
            outputFile << "\t" << num_dimensions_var << " <- " << arr_name + " + 8\n";
            outputFile << "\tstore " << num_dimensions_var << " <- " << to_string(encode(num_dimensions)) << "\n";
            for (int64_t i  = 0; i < num_dimensions; i++){
              string current_dimension_size_var = generate_unique_var_name(p);
              outputFile << "\t" << current_dimension_size_var << " <- " << arr_name + " + " << to_string(16+8*i) << "\n";
              outputFile << "\tstore " << current_dimension_size_var << " <- " << new_array->dimensions[i]->to_string() << "\n";
            }
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
