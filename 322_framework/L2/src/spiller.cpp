#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <algorithm>
#include <iterator>

#include <spiller.h>
#include <utils.h>
#include <functional>

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

  bool check_replace_var(Item* &to_check, string var_name, Var_item* new_var){
    bool is_var = false;
    if (Var_item* var = dynamic_cast<Var_item*>(to_check)){
      if (var->var_name == var_name){
        to_check = new_var;
        is_var = true;
      }
    }
    return is_var;
  }

  Function* get_spilled(Function* f, string var_name, string prefix){
    Function* new_f = new Function();
    int64_t locals = f->locals;
    new_f->name = f->name;
    Address_item* stack_address = new Address_item();
    stack_address->x = new Register_item("rsp");
    stack_address->offset = locals * 8;
    int64_t counter = 0;

    auto instructions = f->instructions;
    for (Instruction* i : instructions){
      vector<reference_wrapper<Item*>> gen = i->generate_gen();
      vector<reference_wrapper<Item*>> kill = i->generate_kill();
      bool gen_has_var = false;
      bool kill_has_var = false;
      Var_item* new_var = create_var(prefix, counter);
      // if (Assignment* assignment = dynamic_cast<Assignment*>(i)){
      //   vector<reference_wrapper<Item*>> gen = assignment->generate_gen2();
      //   for (Item*& g : gen){
      //     // cout << "before: " << g->item_to_string() << endl;
      //     g = new_var;
      //     // cout << "after: " << g->item_to_string() << endl;
      //     // gen_has_var = gen_has_var || check_replace_var(g, var_name, new_var);
      //   }
      //   // gen_has_var = gen_has_var || check_replace_var(assignment->s, var_name, new_var);
      //   // kill_has_var = kill_has_var || check_replace_var(assignment->d, var_name, new_var);
      // }
      // new_f->instructions.push_back(i);

      for (Item*& g : gen){
        // cout << "g: " << g->item_to_string() << "\n";
        gen_has_var = gen_has_var || check_replace_var(g, var_name, new_var);
      }
      for (Item*& k : kill){
        // cout << "k: " << k->item_to_string() << "\n";
        kill_has_var = kill_has_var || check_replace_var(k, var_name, new_var);
      }
      if (gen_has_var){
        new_f->instructions.push_back(create_assignment(stack_address, new_var, "<-"));
      }
      new_f->instructions.push_back(i);
      if (kill_has_var){
        new_f->instructions.push_back(create_assignment(new_var, stack_address, "<-"));
      }
      if (gen_has_var || kill_has_var){
        counter++;
      }
    }

    //   if (Assignment* assignment = dynamic_cast<Assignment*>(i)){
    //     auto d = assignment->d;
    //     auto s = assignment->s;
    //     auto op = assignment->op;
    //     bool has_var = false;
    //     Var_item* new_var = create_var(prefix, counter);
    //     Var_item* var_s = NULL;
    //     Var_item* var_d = NULL;
    //     if ((var_s = dynamic_cast<Var_item*>(s)) && (var_s->var_name == var_name)){
    //       has_var = true;
    //       new_f->instructions.push_back(create_assignment(stack_address, new_var, "<-"));
    //     }
    //     if ((var_d = dynamic_cast<Var_item*>(d)) && (var_d->var_name == var_name)){
    //       if (op == "+=" || op == "-=" || op == "*=" || op == "&=" || op == "<<=" || op == ">>="){
    //         if (has_var){
    //           new_f->instructions.push_back(create_assignment(new_var, new_var, op));
    //         } else {
    //           new_f->instructions.push_back(create_assignment(stack_address, new_var, "<-"));
    //           new_f->instructions.push_back(create_assignment(s, new_var, op));
    //         }
    //       } else {
    //         if (has_var){
    //           new_f->instructions.push_back(create_assignment(new_var, new_var, op));
    //         } else {
    //           new_f->instructions.push_back(create_assignment(s, new_var, op));
    //         }
    //       }
    //       new_f->instructions.push_back(create_assignment(new_var, stack_address, "<-"));
    //       has_var = true;
    //     } else if (has_var){
    //       new_f->instructions.push_back(create_assignment(new_var, d, op));
    //     }
    //     if (has_var){
    //       counter++;
    //     } else {
    //       new_f->instructions.push_back(assignment);
    //     }
    //   } else if (AssignmentCmp* assignmentCmp = dynamic_cast<AssignmentCmp*>(i)){
    //     Comparison c = assignmentCmp->c;
    //
    //     auto var_dest = dynamic_cast<Var_item*>(c.left);
    //     bool is_var_dest = var_dest && var_dest->var_name == var_name;
    //
    //     auto var_src = dynamic_cast<Var_item*>(c.right);
    //     bool is_var_src = var_src && var_src->var_name == var_name;
    //
    //     auto var_ass_dest = dynamic_cast<Var_item*>(assignmentCmp->d);
    //     bool is_var_ass_dest = var_ass_dest && var_ass_dest->var_name == var_name;
    //
    //     Comparison new_c;
    //     if (is_var_src || is_var_dest || is_var_ass_dest) {
    //       Var_item* new_var = create_var(prefix, counter);
    //
    //       if (is_var_src || is_var_dest) {
    //         new_c.cmp_sign = c.cmp_sign;
    //         // Generate read
    //         new_f->instructions.push_back(create_assignment(stack_address, new_var, "<-"));
    //
    //         if (is_var_dest) {
    //           new_c.left = new_var;
    //         } else {
    //           new_c.left = c.left;
    //         }
    //         if (is_var_src) {
    //           new_c.right = new_var;
    //         } else {
    //           new_c.right = c.right;
    //         }
    //       } else {
    //         new_c = c;
    //       }
    //
    //       AssignmentCmp* new_ac = new AssignmentCmp();
    //       new_ac->c = new_c;
    //       new_f->instructions.push_back(new_ac);
    //       if (is_var_ass_dest) {
    //         new_ac->d = new_var;
    //         new_f->instructions.push_back(create_assignment(new_var, stack_address, "<-"));
    //       } else {
    //         new_ac->d = assignmentCmp->d;
    //       }
    //       counter++;
    //     } else {
    //       new_f->instructions.push_back(assignmentCmp);
    //     }
    //   } else if (Cjump* cjump = dynamic_cast<Cjump*>(i)) {
    //     Comparison c = cjump->c;
    //
    //     auto var_dest = dynamic_cast<Var_item*>(c.left);
    //     bool is_var_dest = var_dest && var_dest->var_name == var_name;
    //
    //     auto var_src = dynamic_cast<Var_item*>(c.right);
    //     bool is_var_src = var_src && var_src->var_name == var_name;
    //
    //     Comparison new_c;
    //     if (is_var_src || is_var_dest) {
    //       Var_item* new_var = create_var(prefix, counter);
    //       new_c.cmp_sign = c.cmp_sign;
    //
    //       // Generate read
    //       new_f->instructions.push_back(create_assignment(stack_address, new_var, "<-"));
    //
    //       if (is_var_dest) {
    //         new_c.left = new_var;
    //       } else {
    //         new_c.left = c.left;
    //       }
    //       if (is_var_src) {
    //         new_c.right = new_var;
    //       } else {
    //         new_c.right = c.right;
    //       }
    //       counter++;
    //     } else {
    //       new_c = c;
    //     }
    //
    //     Cjump* new_cjmp = new Cjump();
    //     new_cjmp->c = new_c;
    //     new_cjmp->label1 = cjump->label1;
    //     new_cjmp->label2 = cjump->label2;
    //     new_f->instructions.push_back(new_cjmp);
    //   } else if (Cjump_fallthrough* fallthrough = dynamic_cast<Cjump_fallthrough*>(i)) {
    //     Comparison c = fallthrough->c;
    //
    //     auto var_dest = dynamic_cast<Var_item*>(c.left);
    //     bool is_var_dest = var_dest && var_dest->var_name == var_name;
    //
    //     auto var_src = dynamic_cast<Var_item*>(c.right);
    //     bool is_var_src = var_src && var_src->var_name == var_name;
    //
    //     Comparison new_c;
    //     if (is_var_src || is_var_dest) {
    //       Var_item* new_var = create_var(prefix, counter);
    //       new_c.cmp_sign = c.cmp_sign;
    //
    //       // Generate read
    //       new_f->instructions.push_back(create_assignment(stack_address, new_var, "<-"));
    //
    //       if (is_var_dest) {
    //         new_c.left = new_var;
    //       } else {
    //         new_c.left = c.left;
    //       }
    //       if (is_var_src) {
    //         new_c.right = new_var;
    //       } else {
    //         new_c.right = c.right;
    //       }
    //       counter++;
    //     } else {
    //       new_c = c;
    //     }
    //
    //     Cjump_fallthrough* new_fallthrough = new Cjump_fallthrough();
    //     new_fallthrough->c = new_c;
    //     new_fallthrough->label = fallthrough->label;
    //     new_f->instructions.push_back(new_fallthrough);
    //   } else if (Inc_or_dec* inc_or_dec = dynamic_cast<Inc_or_dec*>(i)){
    //     auto var_w = dynamic_cast<Var_item*>(inc_or_dec->w);
    //     bool is_var_w = var_w && var_w->var_name == var_name;
    //     if (is_var_w){
    //       Var_item* new_var = create_var(prefix, counter);
    //       new_f->instructions.push_back(create_assignment(stack_address, new_var, "<-"));
    //       auto new_inc_or_dec = new Inc_or_dec();
    //       new_inc_or_dec->op = inc_or_dec->op;
    //       new_inc_or_dec->w = new_var;
    //       new_f->instructions.push_back(new_inc_or_dec);
    //       new_f->instructions.push_back(create_assignment(new_var, stack_address, "<-"));
    //       counter++;
    //     } else {
    //       new_f->instructions.push_back(inc_or_dec);
    //     }
    //   } else if (At_arithmetic* at = dynamic_cast<At_arithmetic*>(i)) {
    //     auto var_dest = dynamic_cast<Var_item*>(at->dest);
    //     bool is_var_dest = var_dest && var_dest->var_name == var_name;
    //
    //     auto var_w1 = dynamic_cast<Var_item*>(at->w1);
    //     bool is_var_w1 = var_w1 && var_w1->var_name == var_name;
    //
    //     auto var_w2 = dynamic_cast<Var_item*>(at->w2);
    //     bool is_var_w2 = var_w2 && var_w2->var_name == var_name;
    //
    //     auto new_at = new At_arithmetic();
    //     if (is_var_w1 || is_var_w2 || is_var_dest) {
    //       Var_item* new_var = create_var(prefix, counter);
    //
    //       if (is_var_w1 || is_var_w2) {
    //         // Generate read
    //         new_f->instructions.push_back(create_assignment(stack_address, new_var, "<-"));
    //
    //         if (is_var_w1) {
    //           new_at->w1 = new_var;
    //         } else {
    //           new_at->w1 = at->w1;
    //         }
    //         if (is_var_w2) {
    //           new_at->w2 = new_var;
    //         } else {
    //           new_at->w2 = at->w2;
    //         }
    //       } else {
    //         new_at->w1 = at->w1;
    //         new_at->w2 = at->w2;
    //       }
    //
    //       new_f->instructions.push_back(new_at);
    //       if (is_var_dest) {
    //         new_at->dest = new_var;
    //         new_f->instructions.push_back(create_assignment(new_var, stack_address, "<-"));
    //       } else {
    //         new_at->dest = at->dest;
    //       }
    //       counter++;
    //     } else {
    //       new_f->instructions.push_back(at);
    //     }
    //   } else if (Custom_func_call* cus_func = dynamic_cast<Custom_func_call*>(i)){
    //     Custom_func_call* new_cus_func = new Custom_func_call();
    //     if (Var_item* var = dynamic_cast<Var_item*>(cus_func->u)){
    //       Var_item* new_var = create_var(prefix, counter);
    //       // Generate read
    //       new_f->instructions.push_back(create_assignment(stack_address, new_var, "<-"));
    //       new_cus_func->u = new_var;
    //       counter++;
    //     } else {
    //       new_cus_func->u = cus_func->u;
    //     }
    //     new_f->instructions.push_back(new_cus_func);
    //   } else {
    //     new_f->instructions.push_back(i);
    //   }
    // }


    if (counter==0) {
      new_f->locals = locals;
    } else {
      new_f->locals = locals + 1;
    }
    return new_f;
  }

  void generate_spilled(Program p){
    Function* f = p.functions.front();
    Function* new_f = get_spilled(f, f->var_name, f->prefix);
    L2::print_function(new_f);
  }
}
