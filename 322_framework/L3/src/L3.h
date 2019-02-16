#pragma once

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <functional>
#include <utils.h>

using namespace std;

namespace L3 {
  enum Item_type {VARIABLE, NUMBER, LABEL,  SYSCALL};
  enum Instruction_type {ASSIGN, OP, CMP, LOAD, STORE, GOTO, LABELI, JUMP, RETVOID,
                         RET, CALL, CALLSTORE};

  struct Item {
    L3::Item_type type;
    virtual ~Item() = default;
<<<<<<< HEAD
    virtual std::string to_str(){
=======
    virtual std::string to_string(){
>>>>>>> 403987d247bd043c95e292a772e717992b83b278
      return "";
    }
  };

  struct Variable : Item {
    std::string name;

    Variable(): name() {
      type = L3::Item_type::VARIABLE;
    }
    Variable(std::string x): name(x) {
      type = L3::Item_type::VARIABLE;
    }
    virtual std::string to_string(){
      return name;
    }
  };

  struct Number : Item {
    int64_t n;

    Number(): n() {
      type = L3::Item_type::NUMBER;
    }
    Number(int64_t x): n(x) {
      type = L3::Item_type::NUMBER;
    }

    virtual std::string to_string(){
      return std::to_string(n);
    }
  };

  struct Label : Item {
    std::string name;
    Label(){
      type = L3::Item_type::LABEL;
    }

    virtual std::string to_string(){
      return name;
    }
  };

  struct Sys_call : Item {
    std::string name;
    Sys_call(){
      type = L3::Item_type::SYSCALL;
    }

    virtual std::string to_string(){
      return name;
    }
  };

  struct Instruction {
    L3::Instruction_type type;
    virtual ~Instruction() = default;

    virtual vector<string> generate_read(){
      return {};
    }
    virtual vector<string> generate_defined(){
      return {};
    }
  };

  struct Instruction_assign : Instruction {
    Variable* dest;
    Item* source;
    Instruction_assign(){
      type = L3::Instruction_type::ASSIGN;
    }

    virtual vector<string> generate_read(){
      vector<string> read = {};
      if (source->type == Item_type::VARIABLE) {
        read.push_back(source->to_string());
      }
      return read;
    }

    virtual vector<string> generate_defined(){
      vector<string> kill = {};
      kill.push_back(dest->name);
      return kill;
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

    virtual vector<string> generate_read(){
      vector<string> read = {};
      if (t1->type == Item_type::VARIABLE) {
        read.push_back(t1->to_string());
      }
      if (t2->type == Item_type::VARIABLE) {
        read.push_back(t2->to_string());
      }
      return read;
    }

    virtual vector<string> generate_defined(){
      vector<string> kill = {};
      kill.push_back(dest->name);
      return kill;
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

    virtual vector<string> generate_read(){
      vector<string> read = {};
      if (t1->type == Item_type::VARIABLE) {
        read.push_back(t1->to_string());
      }
      if (t2->type == Item_type::VARIABLE) {
        read.push_back(t2->to_string());
      }
      return read;
    }

    virtual vector<string> generate_defined(){
      vector<string> kill = {};
      kill.push_back(dest->name);
      return kill;
    }
  };

  struct Instruction_load : Instruction {
    Variable* dest;
    Variable* source;
    Instruction_load(){
      type = L3::Instruction_type::LOAD;
    }

    virtual vector<string> generate_read(){
      vector<string> read = {};
      read.push_back(source->name);
      return read;
    }

    virtual vector<string> generate_defined(){
      vector<string> kill = {};
      kill.push_back(dest->name);
      return kill;
    }
  };

  struct Instruction_store : Instruction {
    Variable* dest;
    Item* source;
    Instruction_store(){
      type = L3::Instruction_type::STORE;
    }

    virtual vector<string> generate_read(){
      vector<string> read = {};
      if (source->type == Item_type::VARIABLE) {
        read.push_back(source->to_string());
      }
      return read;
    }

    virtual vector<string> generate_defined(){
      vector<string> kill = {};
      kill.push_back(dest->name);
      return kill;
    }
  };

  struct Instruction_goto : Instruction {
    // br label
    Label* label;
    Instruction_goto(){
      type = L3::Instruction_type::GOTO;
    }

    virtual vector<string> generate_read(){
      return {};
    }

    virtual vector<string> generate_defined(){
      return {};
    }
  };

  struct Instruction_label : Instruction {
    Label* label;
    Instruction_label(){
      type = L3::Instruction_type::LABELI;
    }

    virtual vector<string> generate_read(){
      return {};
    }

    virtual vector<string> generate_defined(){
      return {};
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

    virtual vector<string> generate_read(){
      vector<string> read = {};
      read.push_back(var->name);
      return read;
    }

    virtual vector<string> generate_defined(){
      return {};
    }
  };

  struct Instruction_ret_void : Instruction {
    Instruction_ret_void(){
      type = L3::Instruction_type::RETVOID;
    }

    virtual vector<string> generate_read(){
      return {};
    }

    virtual vector<string> generate_defined(){
      return {};
    }
  };

  struct Instruction_ret : Instruction {
    Item* t;
    Instruction_ret(){
      type = L3::Instruction_type::RET;
    }

    virtual vector<string> generate_read(){
      vector<string> read = {};
      if (t->type == Item_type::VARIABLE) {
        read.push_back(t->to_string());
      }
      return read;
    }

    virtual vector<string> generate_defined(){
      return {};
    }
  };

  struct Instruction_call : Instruction {
    std::vector<Item*> args;
    Item* callee;
    Instruction_call(){
      type = L3::Instruction_type::CALL;
    }

    virtual vector<string> generate_read(){
      vector<string> read = {};
      for (Item* i : args) {
        if (i->type == Item_type::VARIABLE) {
          read.push_back(i->to_string());
        }
      }
      if (callee->type == Item_type::VARIABLE) {
        read.push_back(callee->to_string());
      }
      return read;
    }

    virtual vector<string> generate_defined(){
      return {};
    }
  };

  struct Instruction_call_store : Instruction {
    std::vector<Item*> args;
    Item* callee;
    Variable* dest;
    Instruction_call_store(){
      type = L3::Instruction_type::CALLSTORE;
    }

    virtual vector<string> generate_read(){
      vector<string> read = {};
      for (Item* i : args) {
        if (i->type == Item_type::VARIABLE) {
          read.push_back(i->to_string());
        }
      }
      if (callee->type == Item_type::VARIABLE) {
        read.push_back(callee->to_string());
      }
      return read;
    }

    virtual vector<string> generate_defined(){
      vector<string> defined = {};
      defined.push_back(dest->name);
      return defined;
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

  struct Node {
    Item* value;
    Instruction_type operand_type;
    string op_name;
  };

}
