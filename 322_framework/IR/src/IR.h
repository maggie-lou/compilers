#pragma once

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <functional>
#include <set>

using namespace std;

namespace IR {
  enum Item_type {VARIABLE, NUMBER, LABEL, SYSCALL};
  enum Instruction_type {ASSIGN, OP, LOAD, STORE, LENGTH, CALL, CALLSTORE,
                         ARRAY, LABEL, GOTO, JUMP, RETVOID, RET};

  struct Item {
    IR::Item_type type;
    virtual ~Item() = default;
    virtual string to_string(){
      return "";
    }
  };

  struct Variable : Item {
    string name;

    Variable(): name() {
      type = IR::Item_type::VARIABLE;
    }
    Variable(std::string x): name(x) {
      type = IR::Item_type::VARIABLE;
    }
    virtual string to_string(){
      return name;
    }
  };

  struct Number : Item {
    int64_t n;

    Number(): n() {
      type = IR::Item_type::NUMBER;
    }
    Number(int64_t x): n(x) {
      type = IR::Item_type::NUMBER;
    }

    virtual string to_string(){
      return to_string(n);
    }
  };

  struct Label : Item {
    string name;
    Label(){
      type = IR::Item_type::LABEL;
    }

    virtual string to_string(){
      return name;
    }
  };

  struct Sys_call : Item {
    string name;
    Sys_call(){
      type = IR::Item_type::SYSCALL;
    }

    virtual string to_string(){
      return name;
    }
  };

  struct Instruction {
    IR::Instruction_type type;
  };

  // type var: do nothing

  struct Instruction_assign : Instruction {
    // var <- s
    Variable* dest;
    Item* source;
    Instruction_assign(){
      type = IR::Instruction_type::ASSIGN;
    }
  };

  struct Instruction_op : Instruction {
    // var <- t op t
    Variable* dest;
    string op;
    Item* t1;
    Item* t2;
    Instruction_op(){
      type = IR::Instruction_type::OP;
    }
  };

  struct Instruction_load : Instruction {
    // var <- var([t])+
    Variable* dest;
    Variable* source;
    vector<int64_t> indices;
    Instruction_load(){
      type = IR::Instruction_type::LOAD;
    }
  }

  struct Instruction_store : Instruction {
    // var([t])+ <- s
    Variable* dest;
    Item* source;
    vector<int64_t> indices;
    Instruction_store(){
      type = IR::Instruction_type::STORE;
    }
  }

  struct Intruction_length : Instruction {
    // var <- length var t
    Variable* dest;
    Variable* source;
    Item* dimension;
    Intruction_length(){
      type = IR::Instruction_type::LENGTH;
    }
  }

  struct Instruction_call : Instruction {
    // call callee (args?)
    vector<Item*> args;
    Item* callee;
    Instruction_call(){
      type = IR::Instruction_type::CALL;
    }
  };

  struct Instruction_call_store : Instruction {
    // var <- call callee (args?)
    std::vector<Item*> args;
    Item* callee;
    Variable* dest;
    Instruction_call_store(){
      type = IR::Instruction_type::CALLSTORE;
    }
  };

  struct Instruction_array : Instruction {
    // var <- new Array(args)
    Variable* dest;
    std::vector<Item*> args;
    Instruction_array(){
      type = IR::Instruction_type::ARRAY;
    }
  };

  // treat tuple as a special array
  // struct Instruction_tuple : Instruction {
  //   // var <- new Tuple(t)
  //   Variable* dest;
  //   Item* t;
  //   Instruction_tuple(){
  //     type = IR::Instruction_type::TUPLE;
  //   }
  // };

  struct Instruction_goto : Instruction {
    // br label
    Label* label;
    Instruction_goto(){
      type = IR::Instruction_type::GOTO;
    }
  };

  struct Instruction_label : Instruction {
    // label
    Label* label;
    Instruction_label(){
      type = IR::Instruction_type::LABELI;
    }
  };

  struct Instruction_jump : Instruction  {
    // br var label label
    Variable* var;
    Label* label1;
    Label* label2;
    Instruction_jump(){
      type = IR::Instruction_type::JUMP;
    }
  };

  struct Instruction_ret_void : Instruction {
    // return
    Instruction_ret_void(){
      type = IR::Instruction_type::RETVOID;
    }
  };

  struct Instruction_ret : Instruction {
    // return t
    Item* t;
    Instruction_ret(){
      type = IR::Instruction_type::RET;
    }
  };

  struct Function {
    string name;
    vector<Variable*> arguments;
    vector<Instruction*> instructions;
    string arg_to_string(){
      string ans;
      for (Variable* arg : arguments){
        ans = ans + ", " + arg->name;
      }
      return ans;
    }
  };

  struct Program {
    vector<Function*> functions;
    string longest_var = "";
    int64_t var_count = 0;
  };
}
