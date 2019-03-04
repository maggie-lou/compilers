#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>

#include <code_generation.h>
#include <utils.h>

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

  string generate_unique_label_name(Program &p) {
    string name = p.longest_label + "_" + to_string(p.label_count);
    p.label_count++;
    return name;
  }

  bool is_tuple(string var_name, Function* f) {
    return f->var_definitions[var_name].to_string() == "tuple";
  }


  string get_function_args_string(Function* f) {
    string arg_string = "";
    for (Variable* arg : f->arguments) {
      arg_string = arg_string + arg->type.to_string() + " " + to_IR_var(arg->name) + ", ";
    }
    arg_string = arg_string.substr(0, arg_string.length()-2); // remove trailing comma
    return arg_string;
  }

  void generate_var_encoding_code(ofstream &output_file, string var_name) {
    output_file << "\t" << var_name << " <- " << var_name << " << 1"<< endl;
    output_file << "\t" << var_name << " <- " << var_name << " + 1"<< endl;
  }

  string generate_var_decoding_code(ofstream &output_file, string var_name, Program &p) {
    string new_var_name = generate_unique_var_name(p);
    output_file << "\tint64 " << new_var_name << endl;
    output_file << "\t" << new_var_name << " <- " << var_name << endl;
    output_file << "\t" << new_var_name << " <- " << new_var_name << " >> 1"<< endl;
    return new_var_name;
  }

  void generate_array_checking_code(ofstream &output_file, string arr_name, vector<Item*> indices, Program &p, bool inialize, int64_t &label_counter, int64_t &arr_error_label_counter, bool is_tuple) {
    // Init temp variables
    if (inialize){
      output_file << "\tint64 %array_check" << endl;
      output_file << "\tint64 %attempted_index" << endl;
      output_file << "\tint64 %dimension" << endl;
    }
    arr_error_label_counter += 1;

    // Check array allocation
    output_file << "\t%array_check <- " << to_IR_var(arr_name) << " = 0" << endl;
    output_file << "\tbr %array_check :array_error_index" << to_string(arr_error_label_counter) << " :continue" << to_string(label_counter) << endl;

    // Check valid index acceses
    if (!is_tuple){
      int i = 0;
      for (i=0; i < indices.size(); i++) {
        output_file << "\t:continue" << to_string(label_counter) << endl;
        label_counter += 1;
        output_file << "\t%attempted_index <- " << to_IR_string(indices[i]) << endl;
        output_file << "\t%dimension <- length " << to_IR_var(arr_name) << " " << to_string(i) << endl; // encode
        output_file << "\t%array_check <- %attempted_index < %dimension" << endl;

        output_file << "\tbr %array_check :continue" << to_string(label_counter) << " :array_error_index" << to_string(arr_error_label_counter) << endl;
      }
    }

    output_file << "\t:array_error_index" << to_string(arr_error_label_counter) << endl;
    output_file << "\tcall array-error(" << to_IR_var(arr_name) << ", %attempted_index)" << endl;
    output_file << "\tbr :continue" << to_string(label_counter) << endl;

    output_file << "\t:continue" << to_string(label_counter) << endl; // If all indices are valid, continue on to rest of code
    label_counter += 1;
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
     int64_t label_counter = 0;
     int64_t arr_error_label_counter = 0;
    for (auto f : p.functions){

      outputFile << "define " << f->type.to_string() << " :" << f->name << "(" << get_function_args_string(f) << "){" << endl;

      auto instructions = f->instructions;
      bool startBB = true;
      bool should_init_check_array = true;

      for (Instruction* i : instructions) {
        Instruction_label* is_label = dynamic_cast<Instruction_label*>(i);
        if (startBB){
          if (is_label == NULL){
            outputFile << "\t" << generate_unique_label_name(p) << endl;
          }
          startBB = false;
        } else if (is_label != NULL){
          outputFile << "\tbr " << is_label->label->name << endl;
        }

        vector<Item*> vars_to_decode = to_decode(i);
        vector<string> decoded_vars;
        for (Item* var : vars_to_decode) {
          string decoded_var = generate_var_decoding_code(outputFile, to_IR_string(var), p);
          decoded_vars.push_back(decoded_var);
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
          outputFile << "\t" << to_IR_string(op->dest) << " <- " << decoded_vars[0] << " " << op->op << " " << decoded_vars[1] << "\n";

        } else if (Instruction_load* load = dynamic_cast<Instruction_load*>(i)) {
          string var = load->source->to_string();
          bool is_tuple = f->var_definitions[var].name == "tuple";
          generate_array_checking_code(outputFile, var, load->indices, p, should_init_check_array, label_counter, arr_error_label_counter, is_tuple);
          should_init_check_array = false;
          outputFile << "\t" << to_IR_string(load->dest) << " <- " << to_IR_string(load->source) << to_indices_string(decoded_vars) << endl;

        } else if (Instruction_store* store = dynamic_cast<Instruction_store*>(i)) {
          string var = store->dest->to_string();
          bool is_tuple = f->var_definitions[var].name == "tuple";
          generate_array_checking_code(outputFile, var, store->indices, p, should_init_check_array, label_counter, arr_error_label_counter, is_tuple);
          should_init_check_array = false;
          outputFile << "\t" << to_IR_string(store->dest) << to_indices_string(decoded_vars) << " <- " << to_IR_string(store->source) << endl;

        } else if (Instruction_length* length_i = dynamic_cast<Instruction_length*>(i)) {
          outputFile << "\t" << to_IR_string(length_i->dest) << " <- length " << to_IR_string(length_i->source) << " " << decoded_vars[0] << endl;

        } else if (Instruction_call* call = dynamic_cast<Instruction_call*>(i)) {
          if (f->var_definitions.find(call->callee->to_string()) == f->var_definitions.end()){
            outputFile << "\tcall :" << call->callee->to_string() << to_args_string(call->args) << endl;
          } else {
            outputFile << "\tcall %" << call->callee->to_string() << to_args_string(call->args) << endl;
          }


        } else if (Instruction_call_store* call_store = dynamic_cast<Instruction_call_store*>(i)) {
          if (f->var_definitions.find(call_store->callee->to_string()) == f->var_definitions.end()){
            outputFile << "\t" << to_IR_string(call_store->dest) << " <- call :" << call_store->callee->to_string() << to_args_string(call_store->args) << endl;
          } else {
            outputFile << "\t" << to_IR_string(call_store->dest) << " <- call %" << call_store->callee->to_string() << to_args_string(call_store->args) << endl;
          }

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
          startBB = true;

        } else if (Instruction_label* label_i = dynamic_cast<Instruction_label*>(i)) {
          outputFile << "\t" << label_i->label->name << "\n";

        } else if (Instruction_jump* jump = dynamic_cast<Instruction_jump*>(i)) {
          outputFile << "\tbr " << decoded_vars[0] << " " << jump->label1->name << " " << jump->label2->name << "\n";
          startBB = true;

        } else if (Instruction_ret_void* ret_void = dynamic_cast<Instruction_ret_void*>(i)) {
          outputFile << "\treturn\n";
          startBB = true;

        } else if (Instruction_ret* ret = dynamic_cast<Instruction_ret*>(i)) {
          outputFile << "\treturn " << to_IR_string(ret->t) << "\n";
          startBB = true;
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
