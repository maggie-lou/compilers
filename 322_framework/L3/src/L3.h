#pragma once

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <functional>
#include <utils.h>

namespace L3 {
  enum Item_type {VARIABLE, NUMBER, LABEL,  SYSCALL};
  enum Instruction_type {S, OP, CMP, LOAD, STORE, GOTO, LABELI, JUMP, RETVOID,
                         RET, CALL, CALLSTORE};

  struct Item {
    L3::Item_type type;
    virtual ~Item() = default;
  };

  struct Variable : Item {
    std::string name;

    Variable(): name() {
      type = L3::Item_type::VARIABLE;
    }
    Variable(std::string x): name(x) {
      type = L3::Item_type::VARIABLE;
    }
  };

  struct Number : Item {
    // type = L3::Item_type::NUMBER;
    int64_t n;

    Number(): n() {
      type = L3::Item_type::NUMBER;
    }
    Number(int64_t x): n(x) {
      type = L3::Item_type::NUMBER;
    }
  };

  struct Label : Item {
    std::string name;
    Label(){
      type = L3::Item_type::LABEL;
    }
  };

  struct Sys_call : Item {
    std::string name;
    Sys_call(){
      type = L3::Item_type::SYSCALL;
    }
  };

  struct Node {
    Item* value;
    std::vector<Node*> children;
    std::string operand;
  };

  struct Instruction {
    L3::Instruction_type type;
    virtual ~Instruction() = default;
  };

  struct Instruction_s : Instruction {
    Variable* dest;
    Item* source;
    Instruction_s(){
      type = L3::Instruction_type::S;
    }
  };

  struct Instruction_op : Instruction {
    Variable* dest;
    std::string op;
    Item* t1;
    Item* t2;
    Instruction_op(){
      type = L3::Instruction_type::OP;
    }
  };

  struct Instruction_cmp : Instruction {
    Variable* dest;
    std::string cmp;
    Item* t1;
    Item* t2;
    Instruction_cmp(){
      type = L3::Instruction_type::CMP;
    }
  };

  struct Instruction_load : Instruction {
    Variable* dest;
    Variable* source;
    Instruction_load(){
      type = L3::Instruction_type::LOAD;
    }
  };

  struct Instruction_store : Instruction {
    Variable* dest;
    Item* source;
    Instruction_store(){
      type = L3::Instruction_type::STORE;
    }
  };

  struct Instruction_goto : Instruction {
    // br label
    Label* label;
    Instruction_goto(){
      type = L3::Instruction_type::GOTO;
    }
  };

  struct Instruction_label : Instruction {
    Label* label;
    Instruction_label(){
      type = L3::Instruction_type::LABELI;
    }
  };

  struct Instruction_jump : Instruction  {
    // br var label label
    Variable* var;
    Label* label1;
    Label* label2;
    Instruction_jump(){
      type = L3::Instruction_type::JUMP;
    }
  };

  struct Instruction_ret_void : Instruction {
    Instruction_ret_void(){
      type = L3::Instruction_type::RETVOID;
    }
  };

  struct Instruction_ret : Instruction {
    Item* t;
    Instruction_ret(){
      type = L3::Instruction_type::RET;
    }
  };

  struct Instruction_call : Instruction {
    std::vector<Item*> args;
    Item* callee;
    Instruction_call(){
      type = L3::Instruction_type::CALL;
    }
  };

  struct Instruction_call_store : Instruction {
    std::vector<Item*> args;
    Item* callee;
    Variable* dest;
    Instruction_call_store(){
      type = L3::Instruction_type::CALLSTORE;
    }
  };

  struct Function{
    std::string name;
    std::vector<Variable*> arguments;
    std::vector<Instruction *> instructions;
  };

  struct Program{
    std::vector<Function *> functions;
  };
}
