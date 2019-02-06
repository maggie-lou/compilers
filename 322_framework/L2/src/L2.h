#pragma once

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <functional>

namespace L2 {
  struct Item {
    virtual ~Item() = default;
    virtual std::string item_to_string(){
      return "";
    }
  };

  struct Address_item : Item {
    Item* x;
    int64_t offset;
    virtual std::string item_to_string(){
      return "mem " + x->item_to_string() + " " + std::to_string(offset);
    }
  };

  struct Num_item : Item {
    int64_t n;
    virtual std::string item_to_string(){
      return std::to_string(n);
    }
  };

  struct Label_item : Item {
    std::string label_name;
    virtual std::string item_to_string(){
      return label_name;
    }
  };

  struct Register_item : Item {
    std::string register_name;
    Register_item(): register_name() {}
    Register_item(std::string x): register_name(x) {}
    virtual std::string item_to_string(){
      return register_name;
    }
  };

  struct Var_item : Item {
    std::string var_name;
    virtual std::string item_to_string(){
      return var_name;
    }
  };

  struct Sys_func_item : Item {
    std::string func_name;
    virtual std::string item_to_string(){
      return func_name;
    }
  };

  /*
   * Instruction interface.
   */
  struct Instruction{
    void get_reg_var(Item* &i, std::vector<std::reference_wrapper<Item*>> &original_set){
      auto to_cast = i;
      if (Var_item* var = dynamic_cast<Var_item*>(to_cast)){
        original_set.push_back(i);
      } else if (Register_item* reg = dynamic_cast<Register_item*>(to_cast)){
        if(reg->register_name != "rsp"){
          original_set.push_back(i);
        }
      } else if (Address_item* address = dynamic_cast<Address_item*>(to_cast)){
        get_reg_var(address->x, original_set);
      }
    }

    virtual std::vector<std::reference_wrapper<Item*>> generate_gen(){
      return {};
    }
    virtual std::vector<std::reference_wrapper<Item*>> generate_kill(){
      return {};
    }
  };

  /*
   * Instructions.
   */
  struct Instruction_ret : Instruction{
    int locals;
    int arguments;

    virtual std::vector<std::reference_wrapper<Item*>> generate_gen(){
      // std::vector<Item*> temp_r =
      //   {new Register_item("rax"), new Register_item("r12"), new Register_item("r13"),
      //    new Register_item("r14"), new Register_item("r15"), new Register_item("rbp"),
      //    new Register_item("rbx")};
      // std::vector<std::reference_wrapper<Item*>> gen;
      // for (auto r : temp_r){
      //   gen.push_back(r);
      // }
      return {};
    }

    virtual std::vector<std::reference_wrapper<Item*>> generate_kill(){
      return {};
    }
  };

  /*
   * Function.
   */
  struct Function{
    std::string name;
    int64_t arguments;
    int64_t locals;
    std::vector<Instruction *> instructions;
    std::string prefix;
    std::string var_name;
  };

  struct Comparison {
    Item* left;
    Item* right;
    std::string cmp_sign;

    std::string to_string() {
      return left->item_to_string() + " " + cmp_sign + " " + right->item_to_string();
    }
  };

  /*
   * ++,-- Instruction.
   * w++ | w--
   */
  struct Inc_or_dec : Instruction {
    Item* w;
    std::string op;

    virtual std::vector<std::reference_wrapper<Item*>> generate_gen(){
      return {w};
    }

    virtual std::vector<std::reference_wrapper<Item*>> generate_kill(){
      return {w};
    }
  };

  /*
   * @ Instruction.
   * w @ w w E
   */
  struct At_arithmetic : Instruction {
    Item* dest;
    Item* w1;
    Item* w2;
    int64_t n;

    virtual std::vector<std::reference_wrapper<Item*>> generate_gen(){
      return {w1, w2};
    }

    virtual std::vector<std::reference_wrapper<Item*>> generate_kill(){
      return {dest};
    }
  };

  /*
   * Assignment Instruction.
   * w <- s | w <- mem x M | mem x M <- s
   * w aop t | w sop sx | w sop N |
   * mem x M += t | mem x M -= t | w += mem x M | w -= mem x M |
   * w <- stack-arg M
   */
  struct Assignment : Instruction {
    Item* d;
    Item* s;
    std::string op;

    std::map<std::string, int> m = {
      {"<-", 1}, {"+=", 2}, {"-=", 3}, {"*=", 4},
      {"&=", 5}, {">>=", 6}, {"<<=", 7}, {"<- stack-arg", 8}
    };

    virtual std::vector<std::reference_wrapper<Item*>> generate_gen(){
      std::vector<std::reference_wrapper<Item*>> gen = {};
      get_reg_var(s, gen);
      if (op != "<-" && op != "<- stack-arg") {
        get_reg_var(d, gen);
      }
      return gen;
    }

    virtual std::vector<std::reference_wrapper<Item*>> generate_kill(){
      std::vector<std::reference_wrapper<Item*>> gen = {};
      get_reg_var(d, gen);
      return gen;
    }
  };

  /*
   * Assignment Instruction.
   * w <- t cmp t
   */
  struct AssignmentCmp : Instruction {
    Item* d;
    Comparison c;

    virtual std::vector<std::reference_wrapper<Item*>> generate_gen(){
      std::vector<std::reference_wrapper<Item*>> gen = {};
      get_reg_var(c.left, gen);
      get_reg_var(c.right, gen);
      return gen;
    }

    virtual std::vector<std::reference_wrapper<Item*>> generate_kill(){
      std::vector<std::reference_wrapper<Item*>> gen = {};
      get_reg_var(d, gen);
      return gen;
    }
  };

  /*
   * Cjump Instruction.
   * cjump t cmp t label label
   */
  struct Cjump : Instruction {
    Comparison c;
    std::string label1;
    std::string label2;

    virtual std::vector<std::reference_wrapper<Item*>> generate_gen(){
      std::vector<std::reference_wrapper<Item*>> gen = {};
      get_reg_var(c.left, gen);
      get_reg_var(c.right, gen);
      return gen;
    }

    virtual std::vector<std::reference_wrapper<Item*>> generate_kill(){
      return {};
    }
  };

  /*
   * Cjump Fallthrough Instruction.
   * cjump t cmp t label
   */
  struct Cjump_fallthrough : Instruction {
    Comparison c;
    std::string label;

    virtual std::vector<std::reference_wrapper<Item*>> generate_gen(){
      std::vector<std::reference_wrapper<Item*>> gen = {};
      get_reg_var(c.left, gen);
      get_reg_var(c.right, gen);
      return gen;
    }

    virtual std::vector<std::reference_wrapper<Item*>> generate_kill(){
      return {};
    }
  };

  /*
   * Label Instruction.
   * label
   */
  struct Label_instruction : Instruction {
    std::string label;

    virtual std::vector<std::reference_wrapper<Item*>> generate_gen(){
      return {};
    }

    virtual std::vector<std::reference_wrapper<Item*>> generate_kill(){
      return {};
    }
  };

  /*
   * Goto Instruction.
   * goto label
   */
  struct Goto : Instruction {
    std::string label;

    virtual std::vector<std::reference_wrapper<Item*>> generate_gen(){
      return {};
    }

    virtual std::vector<std::reference_wrapper<Item*>> generate_kill(){
      return {};
    }
  };

  /*
   * Custom function call Instruction.
   * call u N
   */
  struct Custom_func_call : Instruction {
    Item* u;
    int64_t n;
    Item* rax = new Register_item("rax");

    virtual std::vector<std::reference_wrapper<Item*>> generate_gen(){
      // std::vector<Item*> arguments =
      //   {new Register_item("rdi"), new Register_item("rsi"),
      //    new Register_item("rdx"), new Register_item("rcx"),
      //    new Register_item("r8"), new Register_item("r9")};

      std::vector<std::reference_wrapper<Item*>> gen;
      get_reg_var(u, gen);
      for (int i = 0; i < std::min((int)n, 6); i++) {
        // gen.push_back(arguments[i]);
        // gen.push_back(rax);
      }
      return gen;
    }

    virtual std::vector<std::reference_wrapper<Item*>> generate_kill(){
      std::vector<std::reference_wrapper<Item*>> kill;
      // std::vector<std::string> registers = {"rax", "r8", "r9"};
      // for (std::string s : registers){
      //   Item* temp = new Register_item(s);
      //   kill.push_back(temp);
      // }
      // std::vector<Item*> temp_r =
      //   {new Register_item("rax"), new Register_item("r8"), new Register_item("r9"),
      //    new Register_item("r10"), new Register_item("r11"), new Register_item("rcx"),
      //    new Register_item("rdi"), new Register_item("rsi"), new Register_item("rdx")};
      // std::vector<std::reference_wrapper<Item*>> kill;
      // for (Item* r : temp_r){
      //   kill.push_back(r);
      // }
      //
      // for (Item* &r : kill){
      //   if (r == NULL){
      //     std::cout << "has null\n";
      //   }
      //   std::cout << r->item_to_string() <<  "\n";
      // }
      // kill.push_back(rax);
      return {};
    }
  };

  /*
   * System function call Instruction.
   * call u N
   */
  struct System_func_call : Instruction {
    std::string system_func;

    virtual std::vector<std::reference_wrapper<Item*>> generate_gen(){
      Item* temp = new Register_item("rdi");
      std::vector<std::reference_wrapper<Item*>> gen;
      gen.push_back(temp);
      if (system_func != "print"){
        temp = new Register_item("rsi");
        gen.push_back(temp);
      }
      return gen;
    }

    virtual std::vector<std::reference_wrapper<Item*>> generate_kill(){
      std::vector<Item*> temp_r =
        {new Register_item("rax"), new Register_item("r8"), new Register_item("r9"),
         new Register_item("r10"), new Register_item("r11"), new Register_item("rcx"),
         new Register_item("rdi"), new Register_item("rsi"), new Register_item("rdx")};
      std::vector<std::reference_wrapper<Item*>> kill;
      for (auto r : temp_r){
        kill.push_back(r);
      }
      return kill;
    }
  };

  /*
   * Program.
   */
  struct Program{
    std::string entryPointLabel;
    std::vector<Function *> functions;
  };
}
