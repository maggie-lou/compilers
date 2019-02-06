#pragma once

#include <vector>
#include <map>
#include <string>
#include <iostream>

namespace L2 {
  struct Item {
    virtual ~Item() = default;
    virtual std::string item_to_string(){
      return "";
    }
  };

  struct Address_item : Item {
    std::string r;
    int64_t offset;
    virtual std::string item_to_string(){
      return "mem " + r + " " + std::to_string(offset);
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
    void get_reg_var(Item* i, std::vector<std::string> &original_set){
      if (Var_item* var = dynamic_cast<Var_item*>(i)){
        original_set.push_back(var->var_name);
      } else if (Register_item* reg = dynamic_cast<Register_item*>(i)){
        if(reg->register_name != "rsp"){
          original_set.push_back(reg->register_name);
        }
      } else if (Address_item* address = dynamic_cast<Address_item*>(i)){
        if(address->r != "rsp"){
          original_set.push_back(address->r);
        }
      }
    }

    virtual std::vector<std::string> generate_gen(){
      return {};
    }
    virtual std::vector<std::string> generate_kill(){
      return {};
    }
  };

  /*
   * Instructions.
   */
  struct Instruction_ret : Instruction{
    int locals;
    int arguments;

    virtual std::vector<std::string> generate_gen(){
      return {"rax", "r12", "r13", "r14", "r15", "rbp", "rbx"};
    }

    virtual std::vector<std::string> generate_kill(){
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

    virtual std::vector<std::string> generate_gen(){
      return {w->item_to_string()};
    }

    virtual std::vector<std::string> generate_kill(){
      return {w->item_to_string()};
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

    virtual std::vector<std::string> generate_gen(){
      return {w1->item_to_string(), w2->item_to_string()};
    }

    virtual std::vector<std::string> generate_kill(){
      return {dest->item_to_string()};
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

    virtual std::vector<std::string> generate_gen(){
      std::vector<std::string> gen = {};
      get_reg_var(s, gen);
      if (op != "<-" && op != "<- stack-arg") {
        get_reg_var(d, gen);
      }
      return gen;
    }

    virtual std::vector<std::string> generate_kill(){
      std::vector<std::string> gen = {};
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

    virtual std::vector<std::string> generate_gen(){
      std::vector<std::string> gen = {};
      get_reg_var(c.left, gen);
      get_reg_var(c.right, gen);
      return gen;
    }

    virtual std::vector<std::string> generate_kill(){
      std::vector<std::string> gen = {};
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

    virtual std::vector<std::string> generate_gen(){
      std::vector<std::string> gen = {};
      get_reg_var(c.left, gen);
      get_reg_var(c.right, gen);
      return gen;
    }

    virtual std::vector<std::string> generate_kill(){
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

    virtual std::vector<std::string> generate_gen(){
      std::vector<std::string> gen = {};
      get_reg_var(c.left, gen);
      get_reg_var(c.right, gen);
      return gen;
    }

    virtual std::vector<std::string> generate_kill(){
      return {};
    }
  };

  /*
   * Label Instruction.
   * label
   */
  struct Label_instruction : Instruction {
    std::string label;

    virtual std::vector<std::string> generate_gen(){
      return {};
    }

    virtual std::vector<std::string> generate_kill(){
      return {};
    }
  };

  /*
   * Goto Instruction.
   * goto label
   */
  struct Goto : Instruction {
    std::string label;

    virtual std::vector<std::string> generate_gen(){
      return {};
    }

    virtual std::vector<std::string> generate_kill(){
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

    virtual std::vector<std::string> generate_gen(){
      // u, args
      std::vector<std::string> gen;
      std::vector<std::string> arguments = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
      if (Register_item* reg = dynamic_cast<Register_item*>(u)){
        gen.push_back(reg->register_name);
      } else if (Var_item* var = dynamic_cast<Var_item*>(u)){
        gen.push_back(var->var_name);
      }
      for (int i = 0; i < std::min((int)n, 6); i++) {
        gen.push_back(arguments[i]);
      }
      return gen;
    }

    virtual std::vector<std::string> generate_kill(){
      return {"rax", "r8", "r9", "r10", "r11", "rcx", "rdi", "rsi", "rdx"};
    }
  };

  /*
   * System function call Instruction.
   * call u N
   */
  struct System_func_call : Instruction {
    std::string system_func;

    virtual std::vector<std::string> generate_gen(){
      std::vector<std::string> gen = {"rdi"};
      if (system_func != "print"){
        gen.push_back("rsi");
      }
      return gen;
    }

    virtual std::vector<std::string> generate_kill(){
      return {"rax", "r8", "r9", "r10", "r11", "rcx", "rdi", "rsi", "rdx"};
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
