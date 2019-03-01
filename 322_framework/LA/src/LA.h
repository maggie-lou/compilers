#pragma once

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <functional>
#include <set>

using namespace std;

namespace LA {

  struct Item {
    virtual ~Item() = default;
    virtual string to_string(){
      return "";
    }
  };

  struct VariableType {
    string name;

    VariableType(string type): name(type) {
    }

    VariableType() {
      name = "";
    }

    string to_string() {
      return name;
    }
  };

  struct Variable : Item {
    string name;
    VariableType type;

    Variable(): name() {
    }

    Variable(string n): name(n) {
    }

    Variable(string n, VariableType t): name(n), type(t) {
    }

    virtual string to_string(){
      return name;
    }

    string get_type() {
      return type.to_string();
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

  struct Instruction {
    virtual ~Instruction() = default;
  };

  struct Instruction_definition : Instruction {
    VariableType type;
    Variable* var;
  };

  struct Instruction_assign : Instruction {
    // name <- s
    Variable* dest;
    Item* source;
  };

  struct Instruction_op : Instruction {
    // name <- t op t
    Variable* dest;
    string op;
    Item* t1;
    Item* t2;
  };

  struct Instruction_load : Instruction {
    // name <- name([t])+
    Variable* dest;
    Variable* source;
    vector<Item*> indices;
  };

  struct Instruction_store : Instruction {
    // name([t])+ <- s
    Variable* dest;
    Item* source;
    vector<Item*> indices;
  };

  struct Instruction_length : Instruction {
    // name <- length var t
    Variable* dest;
    Variable* source;
    Item* dimension;
  };

  struct Instruction_call : Instruction {
    // name (args?)
    vector<Item*> args;
    Variable* callee;
  };

  struct Instruction_call_store : Instruction {
    // name <- name (args?)
    std::vector<Item*> args;
    Variable* callee;
    Variable* dest;
  };

  struct Instruction_print : Instruction {
    Item* t;
  };

  struct Instruction_array : Instruction {
    // name <- new Array(t...) || name <- new Tuple(t)
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
    VariableType type;
    vector<Variable*> arguments;
    vector<Instruction*> instructions;
    map<string, VariableType> var_definitions;
  };

  struct Program {
    vector<Function*> functions;
    string longest_var = "";
    int64_t var_count = 0;
  };
}
