#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>

#include <code_generator.h>
#include <utils.h>
#include <control_structure_translator.h>
#include <name_binding.h>

using namespace std;

namespace LB {

  string to_indices_string(vector<Item*> indices) {
    string s = "";
    for (Item* i : indices) {
      s = s + "[" + i->to_string() + "]";
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

    LB::binding_name(p);

    for (auto f : p.functions){

      outputFile << f->type.to_string() << " " << f->name << "(" << get_function_args_string(f) << "){" << endl;

      map<Instruction*, string > while_to_cond_label_map;
      auto instructions = LB::add_cond_labels_while(f->instructions, while_to_cond_label_map, p.longest_var, p.var_count);

      map<Instruction*, Instruction_while*> instruction_to_loop_map = LB::map_instructions_to_loop(instructions, f->start_label_to_while_map, f->end_label_to_while_map, while_to_cond_label_map);

      for (Instruction* i : instructions) {

        if (Instruction_definition* def = dynamic_cast<Instruction_definition*>(i)) {
          for (Variable* var : def->vars){
            outputFile << "\t" << def->type.to_string() << " " << var->to_string() << endl;
          }

        } else if (Instruction_assign* assign = dynamic_cast<Instruction_assign*>(i)){
          outputFile << "\t" << assign->dest->to_string() << " <- " << assign->source->to_string() << "\n";

        } else if (Instruction_op* op = dynamic_cast<Instruction_op*>(i)) {
          outputFile << "\t" << op->dest->to_string() << " <- " << op->t1->to_string() << " " << op->op << " " << op->t2->to_string() << "\n";

        } else if (Instruction_if* if_i = dynamic_cast<Instruction_if*>(i)) {
          vector<string> LA_code = LB::generate_branch_code(i, if_i->t1->to_string(), if_i->t2->to_string(), if_i->op, if_i->label1->to_string(), if_i->label2->to_string(), p.longest_var, p.var_count);
          for (string line: LA_code) {
            outputFile << line;
          }

        } else if (Instruction_while* while_i = dynamic_cast<Instruction_while*>(i)) {
          vector<string> LA_code = LB::generate_branch_code(i, while_i->t1->to_string(), while_i->t2->to_string(), while_i->op, while_i->label1->to_string(), while_i->label2->to_string(), p.longest_var, p.var_count);
          for (string line: LA_code) {
            outputFile << line;
          }

        } else if (Instruction_continue* continue_i = dynamic_cast<Instruction_continue*>(i)) {
          Instruction_while* corresponding_while = instruction_to_loop_map[i];
          string top_while_label = while_to_cond_label_map[corresponding_while];
          outputFile << "\tbr "<< top_while_label << "\n";

        } else if (Instruction_break* break_i = dynamic_cast<Instruction_break*>(i)) {
          Instruction_while* corresponding_while = instruction_to_loop_map[i];
          string while_exit_label = corresponding_while->label2->to_string();
          outputFile << "\tbr "<< while_exit_label << "\n";

        } else if (Instruction_load* load = dynamic_cast<Instruction_load*>(i)) {
          outputFile << "\t" << load->dest->to_string() << " <- " << load->source->to_string() << to_indices_string(load->indices) << "\n";

        } else if (Instruction_store* store = dynamic_cast<Instruction_store*>(i)) {
          outputFile << "\t" << store->dest->to_string() << to_indices_string(store->indices) << " <- " << store->source->to_string() << "\n";

        } else if (Instruction_length* length_i = dynamic_cast<Instruction_length*>(i)) {
          outputFile << "\t" << length_i->dest->to_string() << " <- length " << length_i->source->to_string() << " " << length_i->dimension->to_string() << "\n";

        } else if (Instruction_call* call = dynamic_cast<Instruction_call*>(i)) {
          outputFile << "\t" << call->callee->to_string() << to_args_string(call->args) << "\n";

        } else if (Instruction_call_store* call_store = dynamic_cast<Instruction_call_store*>(i)) {
          outputFile << "\t" << call_store->dest->to_string() << " <- " << call_store->callee->to_string() << to_args_string(call_store->args) << "\n";


        } else if (Instruction_print* print = dynamic_cast<Instruction_print*>(i)) {
          outputFile << "\tprint(" << print->t->to_string() << ")\n";

        } else if (Instruction_array* new_array = dynamic_cast<Instruction_array*>(i)) {
          string type = new_array->is_tuple ? "Tuple" : "Array";
          outputFile << "\t" << new_array->dest->to_string() << " <- new " << type << to_args_string(new_array->dimensions) << "\n";

        } else if (Instruction_label* label_i = dynamic_cast<Instruction_label*>(i)) {
          outputFile << "\t" << label_i->label->to_string() << "\n";

        } else if (Instruction_goto* goto_i = dynamic_cast<Instruction_goto*>(i)) {
          outputFile << "\tbr " << goto_i->label->to_string() << "\n";

        } else if (Instruction_ret_void* ret_void = dynamic_cast<Instruction_ret_void*>(i)) {
          outputFile << "\treturn\n";

        } else if (Instruction_ret* ret = dynamic_cast<Instruction_ret*>(i)) {
          outputFile << "\treturn " << ret->t->to_string() << "\n";

        }
      }
      outputFile << "}\n\n";
    }
  }

}
