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
    virtual void name_binding(string &longest_var, int64_t &var_count){

    }
    string generate_unique_var_name(string &longest_var, int64_t &var_count) {
      string name = longest_var + "_" + to_string(var_count);
      var_count++;
      return name;
    }
    Variable* translate_var_name(Variable* v){
      Scope* scope = v->scope;
      while (scope != NULL){
        unordered_map<string, string> var_map = scope->var_map;
        if (var_map.find(v->name) != var_map.end()){
          v->name = var_map[v->name];
          break;
        }
        scope = scope->parent;
      }
      return v;
    }
  };

  struct Instruction_definition : Instruction {
    VariableType type;
    vector<Variable*> vars;
    virtual void name_binding(string &longest_var, int64_t &var_count){
      for (Variable* v: vars){
        Scope* scope = v->scope;
        string new_var_name = generate_unique_var_name(longest_var, var_count);
        scope->var_map.insert( std::pair< std::string, std::string>(v->name, new_var_name ));
        v->name = new_var_name;
      }
    }
  };

  struct Instruction_assign : Instruction {
    // name <- s
    Variable* dest;
    Item* source;
    virtual void name_binding(string &longest_var, int64_t &var_count){
      dest = translate_var_name(dest);
      if (Variable* v = dynamic_cast<Variable*>(source)){
        source = translate_var_name(v);
      }
    }
  };

  struct Instruction_op : Instruction {
    // name <- t op t
    Variable* dest;
    string op;
    Item* t1;
    Item* t2;
    virtual void name_binding(string &longest_var, int64_t &var_count){
      dest = translate_var_name(dest);
      if (Variable* v = dynamic_cast<Variable*>(t1)){
        t1 = translate_var_name(v);
      }
      if (Variable* v = dynamic_cast<Variable*>(t2)){
        t2 = translate_var_name(v);
      }
    }
  };

  struct Instruction_if : Instruction {
    // cond: t op t
    // if (cond) labellabel
    string op;
    Item* t1;
    Item* t2;
    Label* label1;
    Label* label2;
    virtual void name_binding(string &longest_var, int64_t &var_count){
      if (Variable* v = dynamic_cast<Variable*>(t1)){
        t1 = translate_var_name(v);
      }
      if (Variable* v = dynamic_cast<Variable*>(t2)){
        t2 = translate_var_name(v);
      }
    }
  };

  struct Instruction_while : Instruction {
    // cond: t op t
    // while (cond) labellabel
    string op;
    Item* t1;
    Item* t2;
    Label* label1;
    Label* label2;
    virtual void name_binding(string &longest_var, int64_t &var_count){
      if (Variable* v = dynamic_cast<Variable*>(t1)){
        t1 = translate_var_name(v);
      }
      if (Variable* v = dynamic_cast<Variable*>(t2)){
        t2 = translate_var_name(v);
      }
    }
  };

  struct Instruction_continue : Instruction {
    virtual void name_binding(string &longest_var, int64_t &var_count){
    }

  };

  struct Instruction_break : Instruction {
    virtual void name_binding(string &longest_var, int64_t &var_count){
    }

  };

  struct Instruction_load : Instruction {
    // name <- name([t])+
    Variable* dest;
    Variable* source;
    vector<Item*> indices;
    virtual void name_binding(string &longest_var, int64_t &var_count){
      dest = translate_var_name(dest);
      source = translate_var_name(source);
      for (Item* i : indices){
        if (Variable* v = dynamic_cast<Variable*>(i)){
          i = translate_var_name(v);
        }
      }
    }
  };

  struct Instruction_store : Instruction {
    // name([t])+ <- s
    Variable* dest;
    Item* source;
    vector<Item*> indices;
    virtual void name_binding(string &longest_var, int64_t &var_count){
      dest = translate_var_name(dest);
      if (Variable* v = dynamic_cast<Variable*>(source)){
        source = translate_var_name(v);
      }
      for (Item* i : indices){
        if (Variable* v = dynamic_cast<Variable*>(i)){
          i = translate_var_name(v);
        }
      }
    }
  };

  struct Instruction_length : Instruction {
    // name <- length var t
    Variable* dest;
    Variable* source;
    Item* dimension;
    virtual void name_binding(string &longest_var, int64_t &var_count){
      dest = translate_var_name(dest);
      source = translate_var_name(source);
      if (Variable* v = dynamic_cast<Variable*>(dimension)){
        dimension = translate_var_name(v);
      }
    }
  };

  struct Instruction_call : Instruction {
    // name (args?)
    vector<Item*> args;
    Variable* callee;
    virtual void name_binding(string &longest_var, int64_t &var_count){
      callee = translate_var_name(callee);
      for (Item* arg : args){
        if (Variable* v = dynamic_cast<Variable*>(arg)){
          arg = translate_var_name(v);
        }
      }
    }
  };

  struct Instruction_call_store : Instruction {
    // name <- name (args?)
    std::vector<Item*> args;
    Variable* callee;
    Variable* dest;
    virtual void name_binding(string &longest_var, int64_t &var_count){
      callee = translate_var_name(callee);
      dest = translate_var_name(dest);
      for (Item* arg : args){
        if (Variable* v = dynamic_cast<Variable*>(arg)){
          arg = translate_var_name(v);
        }
      }
    }
  };

  struct Instruction_print : Instruction {
    Item* t;
    virtual void name_binding(string &longest_var, int64_t &var_count){
      if (Variable* v = dynamic_cast<Variable*>(t)){
        t = translate_var_name(v);
      }
    }
  };

  struct Instruction_array : Instruction {
    // name <- new Array(t...) || name <- new Tuple(t)
    Variable* dest;
    std::vector<Item*> dimensions;
    bool is_tuple;
    // can i just have a bool tuple here?
    virtual void name_binding(string &longest_var, int64_t &var_count){
      dest = translate_var_name(dest);
      for (Item* dimension : dimensions){
        if (Variable* v = dynamic_cast<Variable*>(dimension)){
          dimension = translate_var_name(v);
        }
      }
    }
  };

  struct Instruction_label : Instruction {
    // label
    Label* label;
    virtual void name_binding(string &longest_var, int64_t &var_count){
    }
  };

  struct Instruction_ret_void : Instruction {
    // return
    virtual void name_binding(string &longest_var, int64_t &var_count){
    }
  };

  struct Instruction_ret : Instruction {
    // return t
    Item* t;
    virtual void name_binding(string &longest_var, int64_t &var_count){
      if (Variable* v = dynamic_cast<Variable*>(t)){
        t = translate_var_name(v);
      }
    }
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
