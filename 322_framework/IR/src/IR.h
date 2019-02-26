#pragma once

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <functional>
#include <set>

using namespace std;

namespace IR {
  enum Variable_type {VOID, INT64, ARRAY, CODE, INVALID};

  struct Item {
    virtual ~Item() = default;
    virtual string to_string(){
      return "";
    }
  };

  struct Variable : Item {
    string name;
    IR::Variable_type type;

    Variable(): name() {
    }

    Variable(std::string n): name(n) {
    }

    Variable(std::string n, IR::Variable_type t): name(n), type(t) {
    }

    virtual string to_string(){
      return name;
    }
  };

  struct Number : Item {
    int64_t n;

    Number(): n() {
    }
    Number(int64_t x): n(x) {
    }

    virtual string to_string(){
      return ::to_string(n);
    }
  };

  struct Label : Item {
    string name;
    virtual string to_string(){
      return name;
    }
  };

  struct Sys_call : Item {
    string name;
    virtual string to_string(){
      return name;
    }
  };

  struct Instruction {
    virtual ~Instruction() = default;
  };

  struct Instruction_definition : Instruction {
    IR::Variable_type type;
    Variable* var;
  };

  struct Instruction_assign : Instruction {
    // var <- s
    Variable* dest;
    Item* source;
  };

  struct Instruction_op : Instruction {
    // var <- t op t
    Variable* dest;
    string op;
    Item* t1;
    Item* t2;
  };

  struct Instruction_load : Instruction {
    // var <- var([t])+
    Variable* dest;
    Variable* source;
    vector<Item*> indices;
  };

  struct Instruction_store : Instruction {
    // var([t])+ <- s
    Variable* dest;
    Item* source;
    vector<Item*> indices;
  };

  struct Instruction_length : Instruction {
    // var <- length var t
    Variable* dest;
    Variable* source;
    Item* dimension;
  };

  struct Instruction_call : Instruction {
    // call callee (args?)
    vector<Item*> args;
    Item* callee;
    string arg_to_string(){
      if (args.empty()){
        return "";
      }
      string ans;
      for (Item* arg : args){
        ans = ans + ", " + arg->to_string();
      }
      return ans.substr(2);
    }
  };

  struct Instruction_call_store : Instruction {
    // var <- call callee (args?)
    std::vector<Item*> args;
    Item* callee;
    Variable* dest;
    string arg_to_string(){
      if (args.empty()){
        return "";
      }
      string ans;
      for (Item* arg : args){
        ans = ans + ", " + arg->to_string();
      }
      return ans.substr(2);
    }
  };

  struct Instruction_array : Instruction {
    // var <- new Array(t...) || var <- new Tuple(t)
    Variable* dest;
    std::vector<Item*> dimensions;
    bool is_tuple;
    // can i just have a bool tuple here?
  };

  struct Instruction_goto : Instruction {
    // br label
    Label* label;
  };

  struct Instruction_label : Instruction {
    // label
    Label* label;
  };

  struct Instruction_jump : Instruction  {
    // br t label label
    Item* check;
    Label* label1;
    Label* label2;
  };

  struct Instruction_ret_void : Instruction {
    // return
  };

  struct Instruction_ret : Instruction {
    // return t
    Item* t;
  };

  struct Function {
    string name;
    IR::Variable_type type;
    vector<Variable*> arguments;
    vector<Instruction*> instructions;
    string arg_to_string(){
      // cout << "in arg to string\n";
      if (arguments.empty()){
        // cout << "empty\n";
        return "";
      }
      // cout << "not empty\n";
      string ans;
      for (Variable* arg : arguments){
        ans = ans + ", " + arg->name;
      }
      // cout << "ans: " << ans << "\n";
      return ans.substr(2);
    }
  };

  struct Program {
    vector<Function*> functions;
    string longest_var = "";
    int64_t var_count = 0;
  };
}
