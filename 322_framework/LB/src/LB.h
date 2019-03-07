#pragma once

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <functional>
#include <set>
#include <unordered_map>

using namespace std;

namespace LB {

  struct Scope {
    Scope* parent;
    vector<Scope*> children;
    unordered_map<string, string> var_map;
  };

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

    bool is_tuple_or_array() {
      return name == "tuple" || name.find("int64[]") != string::npos;
    }
  };

  struct Variable : Item {
    string name;
    VariableType type;
    Scope* scope;

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
    vector<Variable*> vars;
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

  struct Instruction_if : Instruction {
    // cond: t op t
    // if (cond) labellabel
    string op;
    Item* t1;
    Item* t2;
    Label* label1;
    Label* label2;
  };

  struct Instruction_while : Instruction {
    // cond: t op t
    // while (cond) labellabel
    string op;
    Item* t1;
    Item* t2;
    Label* label1;
    Label* label2;
  };

  struct Instruction_continue : Instruction {

  };

  struct Instruction_break : Instruction {

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
  };

  struct Program {
    vector<Function*> functions;
    string longest_var = "";
    int64_t var_count = 0;
    string longest_label = "";
    int64_t label_count = 0;
  };
}
