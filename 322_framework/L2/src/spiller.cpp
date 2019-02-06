#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <algorithm>
#include <iterator>

#include <spiller.h>
#include <utils.h>

using namespace std;

namespace L2{
  Assignment* create_assignment(Item* s, Item* d, string op){
    Assignment* assignment = new Assignment();
    assignment->op = op;
    assignment->d = d;
    assignment->s = s;
    return assignment;
  }

  Var_item* create_var(string prefix, int64_t counter){
    Var_item* var = new Var_item();
    var->var_name = prefix + to_string(counter);
    return var;
  }

  Function* get_spilled(Function* f, string var_name, string prefix){
    Function* new_f = new Function();
    int64_t locals = f->locals;
    new_f->locals = locals + 1;
    new_f->name = f->name;
    Address_item* stack_address = new Address_item();
    stack_address->r = "rsp";
    stack_address->offset = locals * 8;
    int64_t counter = 0;

    auto instructions = f->instructions;
    for (auto i : instructions){
      if (Assignment* assignment = dynamic_cast<Assignment*>(i)){
        auto d = assignment->d;
        auto s = assignment->s;
        auto op = assignment->op;
        bool has_var = false;
        Var_item* new_var = create_var(prefix, counter);
        Var_item* var_s = NULL;
        Var_item* var_d = NULL;
        if ((var_s = dynamic_cast<Var_item*>(s)) && (var_s->var_name == var_name)){
          has_var = true;
          new_f->instructions.push_back(create_assignment(stack_address, new_var, "<-"));
        }
        if ((var_d = dynamic_cast<Var_item*>(d)) && (var_d->var_name == var_name)){
          if (op == "+=" || op == "-=" || op == "*=" || op == "&=" || op == "<<=" || op == ">>="){
            if (has_var){
              new_f->instructions.push_back(create_assignment(new_var, new_var, op));
            } else {
              new_f->instructions.push_back(create_assignment(stack_address, new_var, "<-"));
              new_f->instructions.push_back(create_assignment(s, new_var, op));
            }
          } else {
            if (has_var){
              new_f->instructions.push_back(create_assignment(new_var, new_var, op));
            } else {
              new_f->instructions.push_back(create_assignment(s, new_var, op));
            }
          }
          new_f->instructions.push_back(create_assignment(new_var, stack_address, "<-"));
          has_var = true;
        } else {
          new_f->instructions.push_back(create_assignment(new_var, d, op));
        }
        if (has_var){
          counter++;
        } else {
          new_f->instructions.push_back(assignment);
        }
      } else if (AssignmentCmp* assignmentCmp = dynamic_cast<AssignmentCmp*>(i)){
        Comparison c = assignmentCmp->c;

        auto var_dest = dynamic_cast<Var_item*>(c.left);
        bool is_var_dest = var_dest && var_dest->var_name == var_name;

        auto var_src = dynamic_cast<Var_item*>(c.right);
        bool is_var_src = var_src && var_src->var_name == var_name;

        auto var_ass_dest = dynamic_cast<Var_item*>(assignmentCmp->d);
        bool is_var_ass_dest = var_ass_dest && var_ass_dest->var_name == var_name;

        Comparison new_c;
        if (is_var_src || is_var_dest || is_var_ass_dest) {
          Var_item* new_var = create_var(prefix, counter);

          if (is_var_src || is_var_dest) {
            new_c.cmp_sign = c.cmp_sign;
            // Generate read
            new_f->instructions.push_back(create_assignment(stack_address, new_var, "<-"));

            if (is_var_dest) {
              new_c.left = new_var;
            } else {
              new_c.left = c.left;
            }
            if (is_var_src) {
              new_c.right = new_var;
            } else {
              new_c.right = c.right;
            }
          } else {
            new_c = c;
          }

          AssignmentCmp* new_ac = new AssignmentCmp();
          new_ac->c = new_c;
          new_f->instructions.push_back(new_ac);
          if (is_var_ass_dest) {
            new_ac->d = new_var;
            new_f->instructions.push_back(create_assignment(new_var, stack_address, "<-"));
          } else {
            new_ac->d = assignmentCmp->d;
          }
          counter++;
        } else {
          new_f->instructions.push_back(assignmentCmp);
        }
      } else if (Cjump* cjump = dynamic_cast<Cjump*>(i)) {
        Comparison c = cjump->c;

        auto var_dest = dynamic_cast<Var_item*>(c.left);
        bool is_var_dest = var_dest && var_dest->var_name == var_name;

        auto var_src = dynamic_cast<Var_item*>(c.right);
        bool is_var_src = var_src && var_src->var_name == var_name;

        Comparison new_c;
        if (is_var_src || is_var_dest) {
          Var_item* new_var = create_var(prefix, counter);
          new_c.cmp_sign = c.cmp_sign;

          // Generate read
          new_f->instructions.push_back(create_assignment(stack_address, new_var, "<-"));

          if (is_var_dest) {
            new_c.left = new_var;
          } else {
            new_c.left = c.left;
          }
          if (is_var_src) {
            new_c.right = new_var;
          } else {
            new_c.right = c.right;
          }
          counter++;
        } else {
          new_c = c;
        }

        Cjump* new_cjmp = new Cjump();
        new_cjmp->c = new_c;
        new_cjmp->label1 = cjump->label1;
        new_cjmp->label2 = cjump->label2;
        new_f->instructions.push_back(new_cjmp);
      } else if (Cjump_fallthrough* fallthrough = dynamic_cast<Cjump_fallthrough*>(i)) {
        Comparison c = fallthrough->c;

        auto var_dest = dynamic_cast<Var_item*>(c.left);
        bool is_var_dest = var_dest && var_dest->var_name == var_name;

        auto var_src = dynamic_cast<Var_item*>(c.right);
        bool is_var_src = var_src && var_src->var_name == var_name;

        Comparison new_c;
        if (is_var_src || is_var_dest) {
          Var_item* new_var = create_var(prefix, counter);
          new_c.cmp_sign = c.cmp_sign;

          // Generate read
          new_f->instructions.push_back(create_assignment(stack_address, new_var, "<-"));

          if (is_var_dest) {
            new_c.left = new_var;
          } else {
            new_c.left = c.left;
          }
          if (is_var_src) {
            new_c.right = new_var;
          } else {
            new_c.right = c.right;
          }
          counter++;
        } else {
          new_c = c;
        }

        Cjump_fallthrough* new_fallthrough = new Cjump_fallthrough();
        new_fallthrough->c = new_c;
        new_fallthrough->label = fallthrough->label;
        new_f->instructions.push_back(new_fallthrough);
      }
    }
    return new_f;
  }

  void generate_spilled(Program p){
    Function* f = p.functions.front();
    Function* new_f = get_spilled(f, f->var_name, f->prefix);
    L2::print_function(new_f);
  }
}
