#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include <code_generator.h>

using namespace std;

namespace IR{

  string generate_unique_var_name(Program &p) {
    string name = p.longest_var + "_" + to_string(p.var_count);
    p.var_count++;
    return name;
  }

  bool is_tuple(string var_name, Function* f) {
    return f->var_definitions[var_name] == IR::Variable_type::TUPLE;
  }

  string get_offset(Program &p, string arr_name, ofstream &outputFile, vector<Item*> indices, bool is_tuple){
    if (is_tuple) {
      string offset_addr_var = generate_unique_var_name(p);
      string offset = indices[0]->to_string();

      // Generate size of offset
      outputFile << "\t" << offset_addr_var << " <- " << offset << endl;
      outputFile << "\t" << offset_addr_var << " <- " << offset_addr_var << " * 8" << endl;
      // Offset by element used to store size of array
      outputFile << "\t" << offset_addr_var << " <- " << offset_addr_var << " + 8" << endl;

      // Add offset to array address
      outputFile << "\t" << offset_addr_var << " <- " << arr_name << " + " << offset_addr_var << endl;
      return offset_addr_var;
    } else {
      int64_t num_indices = indices.size();
      string addr = generate_unique_var_name(p);
	    string product_dimensions = generate_unique_var_name(p);

	    outputFile << "\t" << addr << " <- 0\n";
	    outputFile << "\t" << product_dimensions << " <- 1\n";

      // Define variables
      string base_offset = generate_unique_var_name(p);
      string sum_products = generate_unique_var_name(p);
      string product_index_remaining_dims = generate_unique_var_name(p);
      string past_dim_addr = generate_unique_var_name(p);
      string past_dim_var = generate_unique_var_name(p);
      string final_addr = generate_unique_var_name(p);

      outputFile << "\t" << sum_products << " <- " << 0;

	    // get the size of each dimension
	    for (int64_t i = 0; i < indices.size(); i++){

        // Load current index
        outputFile << "\t" << product_index_remaining_dims << " <- " << indices[i]->to_string() << "\n";

        // Multiply by all past dimensions
        for (int past_dim = i+1; past_dim < indices.size(); past_dim++) {
          outputFile << "\t" << past_dim_addr << " <- " << arr_name << " + " << to_string(16 + past_dim * 8) << "\n";
          outputFile << "\t" << past_dim_var << " <- load " << past_dim_addr << "\n";
          outputFile << "\t" << product_index_remaining_dims << " <- " << product_index_remaining_dims << " * " << past_dim_var << "\n";
        }

        outputFile << "\t" << sum_products << " <- " << sum_products << " + " << product_index_remaining_dims << endl;
      }

      // Multiply by 8 for memory elements
      outputFile << "\t" << sum_products << " <- " << sum_products << " * 8 \n";

      // Add base offset
      outputFile << "\t" << base_offset << " <- " << to_string(16 + indices.size() * 8) << "\n";
      outputFile << "\t" << final_addr << " <- " << sum_products + base_offset << "\n";

	    return final_addr;
    }
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
          string addr = get_offset(p, source, outputFile, load->indices, is_tuple(source, f));
          outputFile << "\t" << load->dest->to_string() << " <- load " << addr << "\n";

        } else if (Instruction_store* store = dynamic_cast<Instruction_store*>(i)) {
          string dest = store->dest->to_string();
          string addr = get_offset(p, dest, outputFile, store->indices, is_tuple(dest, f));
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
            outputFile << "\t" << linearized_len_var << " <- " << linearized_len_var <<  " + " << to_string((num_dimensions+1) * 8) << "\n";
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
