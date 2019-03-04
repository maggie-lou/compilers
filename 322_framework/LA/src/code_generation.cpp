#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include <code_generation.h>

using namespace std;

namespace LA {

  string to_IR_var(string name) {
    return "%" + name;
  }

  string to_IR_string(Item* i) {
    if (Variable* var = dynamic_cast<Variable*>(i)) {
      return to_IR_var(i->to_string());
    } else {
      return i->to_string();
    }
  }

  string to_indices_string(vector<Item*> indices) {
    string s = "";
    for (Item* i : indices) {
      s = s + "[" + to_IR_string(i) + "]";
    }
    return s;
  }

  string to_args_string(vector<Item*> args) {
    string s = "(";
    for (Item* i : args) {
      s = s + to_IR_string(i) + ",";
    }
    s = s.substr(0, s.length() - 1);
    s = s + ")";
    return s;
  }


  string generate_unique_var_name(Program &p) {
    string name = to_IR_var(p.longest_var + "_" + to_string(p.var_count));
    p.var_count++;
    return name;
  }

  bool is_tuple(string var_name, Function* f) {
    return f->var_definitions[var_name].to_string() == "tuple";
  }


  string get_function_args_string(Function* f) {
    string arg_string = "";
    for (Variable* arg : f->arguments) {
      arg_string = arg_string + arg->type.to_string() + " " + arg->name + ", ";
    }
    arg_string.substr(0, arg_string.length()-1); // remove trailing comma
    return arg_string;
  }

  void generate_var_encoding_code(ofstream &output_file, string var_name) {
    output_file << "\t" << var_name << " <- " << var_name << "<< 1"<< endl;
    output_file << "\t" << var_name << " <- " << var_name << "+ 1"<< endl;
  }

  void generate_var_decoding_code(ofstream &output_file, string var_name) {
    output_file << "\t" << var_name << " <- " << var_name << ">> 1"<< endl;
  }

  void generate_array_checking_code(ofstream &output_file, string arr_name, vector<Item*> indices) {
    // Init temp variables
    output_file << "\tint64 %array_check" << endl;
    output_file << "\tint64 %attempted_index" << endl;
    output_file << "\tint64 %array_check" << endl;
    output_file << "\tint64 %dimension" << endl;

    // Check array allocation
    output_file << "\t%array_check <- " << to_IR_var(arr_name) << " = 0" << endl;
    output_file << "\tbr %array_check :continue0 :array_error_alloc" << endl;

    // Check valid index acceses
    int i = 0;
    for (i=0; i < indices.size(); i++) {
      output_file << "\t:continue" << to_string(i) << endl;
      output_file << "\t%attempted_index <- " << to_IR_string(indices[i]) << endl;
      generate_var_encoding_code(output_file, "%attempted_index");
      output_file << "\t%dimension <- length " << to_IR_var(arr_name) << " " << to_string(i) << endl; // encode
      output_file << "\t%array_check <- %attempted_index < %dimension" << endl;
      output_file << "\tbr %array_check :continue" << to_string(i+1) << " :array_error_index" << endl;
    }

    output_file << "\t:array_error_index" << endl;
    output_file << "\tcall array-error(" << to_IR_var(arr_name) << ", %attempted_index)" << endl;

    output_file << "\t:array_error_alloc" << endl;
    output_file << "\tcall array-error(0,0)" << endl;

    output_file << "\t:continue" << to_string(i) << endl; // If all indices are valid, continue on to rest of code
  }

  vector<Item*> to_decode(Instruction* i) {
    vector<Item*> to_decode;
    if (Instruction_op* op = dynamic_cast<Instruction_op*>(i)) {
      to_decode.push_back(op->t1);
      to_decode.push_back(op->t2);
    } else if (Instruction_load* load = dynamic_cast<Instruction_load*>(i)) {
      for (Item* index : load->indices) {
        to_decode.push_back(index);
      }
    } else if (Instruction_store* store = dynamic_cast<Instruction_store*>(i)) {
      for (Item* index : store->indices) {
        to_decode.push_back(index);
      }
    } else if (Instruction_length* length_i = dynamic_cast<Instruction_length*>(i)) {
      to_decode.push_back(length_i->dimension);
    } else if (Instruction_jump* jump = dynamic_cast<Instruction_jump*>(i)) {
      to_decode.push_back(jump->check);
    }

    return to_decode;
  }

  vector<Item*> to_encode(Instruction* i) {
    vector<Item*> to_encode;
    if (Instruction_op* op = dynamic_cast<Instruction_op*>(i)) {
      to_encode.push_back(op->dest);
    }

    return to_encode;
  }

  void generate_code(Program p){
    /*
     * Open the output file.
     */
    ofstream outputFile;
    outputFile.open("prog.IR");

    /*
     * Generate target code
     */
    for (auto f : p.functions){
      outputFile << f->name << f->type.to_string() << "(" << get_function_args_string(f) << "){" << endl;

      auto instructions = f->instructions;
      for (Instruction* i : instructions) {
        vector<Item*> vars_to_decode = to_decode(i);
        for (Item* var : vars_to_decode) {
          generate_var_decoding_code(outputFile, to_IR_string(var));
        }

        if (Instruction_definition* def = dynamic_cast<Instruction_definition*>(i)) {
          outputFile << "\t" << def->type.to_string() << " " << to_IR_string(def->var) << endl;

          // Set unallocated arrays to 0
          if (def->type.is_tuple_or_array()) {
            outputFile << "\t"<< to_IR_string(def->var) << " <- 0" << endl;
          }
        } else if (Instruction_assign* assign = dynamic_cast<Instruction_assign*>(i)){
          outputFile << "\t" << to_IR_string(assign->dest) << " <- " << to_IR_string(assign->source) << "\n";

        } else if (Instruction_op* op = dynamic_cast<Instruction_op*>(i)) {
          outputFile << "\t" << to_IR_string(op->dest) << " <- " << to_IR_string(op->t1) << " " << op->op << " " << to_IR_string(op->t2) << "\n";

        } else if (Instruction_load* load = dynamic_cast<Instruction_load*>(i)) {
          generate_array_checking_code(outputFile, load->source->to_string(), load->indices);
          outputFile << "\t" << to_IR_string(load->dest) << " <- " << to_IR_string(load->source) << to_indices_string(load->indices) << endl;

        } else if (Instruction_store* store = dynamic_cast<Instruction_store*>(i)) {
          generate_array_checking_code(outputFile, store->dest->to_string(), store->indices);
          outputFile << "\t" << to_IR_string(store->dest) << to_indices_string(store->indices) << " <- " << to_IR_string(store->source) << endl;

        } else if (Instruction_length* length_i = dynamic_cast<Instruction_length*>(i)) {
          outputFile << "\t" << to_IR_string(length_i->dest) << " <- length " << to_IR_string(length_i->source) << " " << to_IR_string(length_i->dimension) << endl;

        } else if (Instruction_call* call = dynamic_cast<Instruction_call*>(i)) {
          outputFile << "\tcall " << to_IR_string(call->callee) << to_args_string(call->args) << endl;

        } else if (Instruction_call_store* call_store = dynamic_cast<Instruction_call_store*>(i)) {
          outputFile << "\t" << to_IR_string(call_store->dest) << " <- call " << to_IR_string(call_store->callee) << to_args_string(call_store->args) << endl;

        } else if (Instruction_print* print = dynamic_cast<Instruction_print*>(i)) {
          outputFile << "\tcall print("<< to_IR_string(print->t) << ")" << endl;
        } else if (Instruction_array* new_array = dynamic_cast<Instruction_array*>(i)) {
          if (new_array->is_tuple) {
            outputFile << "\t" << to_IR_string(new_array->dest) << " <- new Tuple" << to_args_string(new_array->dimensions) << endl;
          } else {
            outputFile << "\t" << to_IR_string(new_array->dest) << " <- new Array" << to_args_string(new_array->dimensions) << endl;
          }

        } else if (Instruction_goto* goto_i = dynamic_cast<Instruction_goto*>(i)) {
          outputFile << "\tbr " << goto_i->label->name << "\n";

        } else if (Instruction_label* label_i = dynamic_cast<Instruction_label*>(i)) {
          outputFile << "\t" << label_i->label->name << "\n";

        } else if (Instruction_jump* jump = dynamic_cast<Instruction_jump*>(i)) {
          outputFile << "\tbr " << to_IR_string(jump->check) << " " << jump->label1->name << " " << jump->label2->name << "\n";

        } else if (Instruction_ret_void* ret_void = dynamic_cast<Instruction_ret_void*>(i)) {
          outputFile << "\treturn\n";

        } else if (Instruction_ret* ret = dynamic_cast<Instruction_ret*>(i)) {
          outputFile << "\treturn " << to_IR_string(ret->t) << "\n";
        }
        vector<Item*> vars_to_encode = to_encode(i);
        for (Item* var : vars_to_encode) {
          generate_var_encoding_code(outputFile, to_IR_string(var));
        }
      }
      outputFile << "}\n\n";
    }
  }

}
