#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <ctype.h>

#include <iostream>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include <IR.h>
#include <parser.h>
#include <utils.h>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace IR {

  /*
   * Data required to parse
   */
  std::vector<Item*> parsed_items;
  std::vector<std::string> parsed_operations;
  std::vector<IR::Variable_type> parsed_variable_types;
  std::vector<std::vector<Item*>> parsed_args;

  struct comment:
    pegtl::disable<
      TAOCPP_PEGTL_STRING( "//" ),
      pegtl::until< pegtl::eolf >
    > {};

  struct seps:
    pegtl::star<
      pegtl::sor<
        pegtl::ascii::space,
        comment
      >
    > {};

  struct name:
    pegtl::seq<
      pegtl::plus<
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >
        >
      >,
      pegtl::star<
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >,
          pegtl::digit
        >
      >
    > {};

  struct label:
    pegtl::seq<
      pegtl::one<':'>,
      name
    > {};

  struct number:
    pegtl::seq<
      pegtl::opt<
        pegtl::sor<
          pegtl::one< '-' >,
          pegtl::one< '+' >
        >
      >,
      pegtl::plus<
        pegtl::digit
      >
    >{};

  struct var:
    pegtl::seq<
      pegtl::one< '%' >,
      name
    > { };

  struct Label_rule:
    label {};

  struct Number_rule:
    number {};

  struct Var_rule:
    var {};

  struct Function_name_rule:
    label {};

  struct Type_rule:
    pegtl::sor<
      pegtl::string<'t', 'u', 'p', 'l', 'e'>,
      pegtl::string<'c', 'o', 'd', 'e'>,
      pegtl::seq<
        pegtl::string<'i', 'n', 't', '6', '4'>,
        pegtl::star<
          pegtl::string<'[', ']'>
        >
      >
    > {};

  struct Vars_rule:
    pegtl::seq<
      pegtl::star<
        Type_rule,
        seps,
        var,
        seps
      >,
      pegtl::star<
        seps,
        pegtl::one<','>,
        seps,
        Type_rule,
        seps,
        var
      >
    > {};

  struct T_rule:
    pegtl::sor<
      Var_rule,
      Number_rule
    > {};

  struct S_rule:
    pegtl::sor<
      T_rule,
      Label_rule
    > {};

  struct Op_rule:
    pegtl::sor<
      pegtl::one<'+'>,
      pegtl::one<'-'>,
      pegtl::one<'*'>,
      pegtl::one<'&'>,
      pegtl::string<'<','<'>,
      pegtl::string<'>','>'>,
      pegtl::string<'<','='>,
      pegtl::one<'='>,
      pegtl::string<'>','='>,
      pegtl::one<'<'>,
      pegtl::one<'>'>
    > {};

  struct U_rule:
    pegtl::sor<
      Var_rule,
      Label_rule
    > {};

  struct Callee_other_rule:
    pegtl::sor<
      pegtl::string<'p','r','i','n','t'>,
      pegtl::string<'a','r','r','a','y','-','e','r','r','o','r'>
    > {};

  struct Callee_rule:
    pegtl::sor<
      U_rule,
      Callee_other_rule
    > {};

  struct Args_t_rule:
    pegtl::sor<
      var,
      number,
      seps
    > {};

  struct Args_rule:
    pegtl::seq<
      Args_t_rule,
      seps,
      pegtl::star<
        seps,
        pegtl::one<','>,
        seps,
        Args_t_rule
      >
    > {};

  struct Function_type_rule:
    pegtl::sor<
      Type_rule,
      pegtl::string<'v', 'o', 'i', 'd'>
    > {};

  struct Instruction_definition_rule:
    pegtl::seq<
      Type_rule,
      seps,
      Var_rule
    > {};

  struct Instruction_assign_rule:
    pegtl::seq<
      Var_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      S_rule
    > {};

  struct Instruction_op_rule:
    pegtl::seq<
      Var_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      T_rule,
      seps,
      Op_rule,
      seps,
      T_rule
    > {};

  struct Array_access_rule:
    pegtl::seq<
      pegtl::one<'['>,
      Args_t_rule,
      pegtl::one<']'>
    > {};

  struct Array_accesses_rule:
    pegtl::seq<
      pegtl::plus<
        Array_access_rule
      >
    > {};

  struct Instruction_load_rule:
    pegtl::seq<
      Var_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      Var_rule,
      Array_accesses_rule
    > {};

  struct Instruction_store_rule:
    pegtl::seq<
      Var_rule,
      Array_accesses_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      S_rule
    > {};

  struct Instruction_length_rule:
    pegtl::seq<
      Var_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      pegtl::string<'l','e','n','g','t','h'>,
      seps,
      Var_rule,
      seps,
      T_rule
    > {};

  struct Instruction_array_rule:
    pegtl::seq<
      Var_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      pegtl::string<'n','e','w'>,
      seps,
      pegtl::sor<
        pegtl::string<'A','r','r','a','y'>,
        pegtl::string<'T','u','p','l','e'>
      >,
      seps,
      pegtl::one<'('>,
      seps,
      Args_rule,
      seps,
      pegtl::one<')'>
    > {};

  struct Instruction_goto_rule:
    pegtl::seq<
      pegtl::string<'b','r'>,
      seps,
      Label_rule
    > {};

  struct Instruction_label_rule:
    pegtl::seq<
      Label_rule
    > {};

  struct Instruction_jump_rule:
    pegtl::seq<
      pegtl::string<'b','r'>,
      seps,
      T_rule,
      seps,
      Label_rule,
      seps,
      Label_rule
    > {};

  struct Instruction_ret_void_rule:
    pegtl::string<'r','e','t','u','r','n'> {};


  struct Instruction_ret_rule:
    pegtl::seq<
      pegtl::string<'r','e','t','u','r','n'>,
      seps,
      T_rule
    > {};

  struct Instruction_call_rule:
    pegtl::seq<
      pegtl::string<'c','a','l','l'>,
      seps,
      Callee_rule,
      seps,
      pegtl::one<'('>,
      seps,
      Args_rule,
      seps,
      pegtl::one<')'>
    > {};

  struct Instruction_call_store_rule:
    pegtl::seq<
      Var_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      pegtl::string<'c','a','l','l'>,
      seps,
      Callee_rule,
      seps,
      pegtl::one<'('>,
      seps,
      Args_rule,
      seps,
      pegtl::one<')'>
    > {};

  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<Instruction_load_rule>, Instruction_load_rule >,
      pegtl::seq< pegtl::at<Instruction_store_rule>, Instruction_store_rule >,
      pegtl::seq< pegtl::at<Instruction_definition_rule>, Instruction_definition_rule >,
      pegtl::seq< pegtl::at<Instruction_op_rule>, Instruction_op_rule >,
      pegtl::seq< pegtl::at<Instruction_assign_rule>, Instruction_assign_rule >,
      pegtl::seq< pegtl::at<Instruction_length_rule>, Instruction_length_rule >,
      pegtl::seq< pegtl::at<Instruction_array_rule>, Instruction_array_rule >,
      pegtl::seq< pegtl::at<Instruction_call_store_rule>, Instruction_call_store_rule >,
      pegtl::seq< pegtl::at<Instruction_call_rule>, Instruction_call_rule >
    > { };

  struct Instructions_rule:
    pegtl::star<
      pegtl::seq<
        seps,
        Instruction_rule,
        seps
      >
    > { };

  struct Block_ending_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<Instruction_jump_rule>, Instruction_jump_rule >,
      pegtl::seq< pegtl::at<Instruction_goto_rule>, Instruction_goto_rule >,
      pegtl::seq< pegtl::at<Instruction_ret_rule>, Instruction_ret_rule >,
      pegtl::seq< pegtl::at<Instruction_ret_void_rule>, Instruction_ret_void_rule >
    > { };

  struct Basic_block_rule:
    pegtl::seq<
      Instruction_label_rule,
      seps,
      Instructions_rule,
      seps,
      Block_ending_rule
    > { };

  struct Basic_blocks_rule:
    pegtl::plus<
      pegtl::seq<
        seps,
        Basic_block_rule,
        seps
      >
    > { };

  struct Function_rule:
    pegtl::seq<
      pegtl::string<'d','e','f','i','n','e'>,
      seps,
      Function_type_rule,
      seps,
      Function_name_rule,
      seps,
      pegtl::one<'('>,
      seps,
      Vars_rule,
      seps,
      pegtl::one<')'>,
      seps,
      pegtl::one<'{'>,
      seps,
      Basic_blocks_rule,
      seps,
      pegtl::one<'}'>
    > {};

  struct Program_rule:
    pegtl::plus<
      seps,
      Function_rule,
      seps
    > {};


  /*
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < Label_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      // cout << "in label rule\n";
      auto i = new Label();
      i->name = in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Number_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Number();
      // cout << "in number rule\n";
      i->n = std::stoll(in.string());
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Var_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Variable();
      // cout << "in var rule\n";
      i->name = in.string();
      parsed_items.push_back(i);
      if (p.longest_var.length() < in.string().length()){
        p.longest_var = in.string();
      }
    }
  };

  template<> struct action < Op_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    // cout << "in op rule\n";
      parsed_operations.push_back(in.string());
    }
  };

  template<> struct action < Type_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      // cout << "in type rule\n";
      IR::Variable_type type = IR::str_to_variable_type(in.string());
      parsed_variable_types.push_back(type);
    }
  };

  template<> struct action < Callee_other_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Sys_call();
      // cout << "in callee other rule\n";
      i->name = in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Function_type_rule > {
    template< typename Input >
      static void apply( const Input & in, Program & p){
        auto f = new Function();
        std::string type = in.string();
        // cout << "in function type rule\n";

        if (type == "tuple" || type.find("int64[]") != std::string::npos) {
          f->type = IR::Variable_type::ARRAY;
        } else if (type == "code") {
          f->type = IR::Variable_type::CODE;
        } else if (type == "void") {
          f->type = IR::Variable_type::VOID;
        } else if (type == "int64") {
          f->type = IR::Variable_type::INT64;
        }
        p.functions.push_back(f);
      }
  };

  template<> struct action < Function_name_rule > {
    template< typename Input >
      static void apply( const Input & in, Program & p){
        Function* f = p.functions.back();
        // cout << "in function name rule\n";
        f->name = in.string();
      }
  };

  template<> struct action < Array_accesses_rule > {
    template< typename Input >
      static void apply( const Input & in, Program & p){
        std::vector<Item*> array_accesses;
        // cout << "in array accesses rule\n";
        std::string accesses_str = in.string();
        std::vector<std::string> accesses_strs;
        size_t index = 0;
        while ((index = accesses_str.find(']')) != std::string::npos) {
          accesses_strs.push_back(accesses_str.substr(0, index));
          accesses_str.erase(0, index+1);
        }
        for (std::string a : accesses_strs){
          a = a.substr(1);
          // cout << "a: " << a << "end\n";
          if (IR::is_int(a)){
            Number* n = new Number(std::stoll(a));
            // cout << "parsed number " << a << "\n";
            array_accesses.push_back(n);
          } else {
            Variable* v = new Variable(a);
            // cout << "parsed var " << v->name << "\n";
            array_accesses.push_back(v);
          }
        }
        parsed_args.push_back(array_accesses);

      }
  };

  template<> struct action < Vars_rule > {
    template< typename Input >
      static void apply( const Input & in, Program & p){
        std::string vars_str = in.string();
        if (vars_str.length() > 0){
          Function* f = p.functions.back();
          // cout << "in vars rule\n";

          // Tokenize vars to parse
          std::istringstream iss(vars_str);
          std::string var;
          while (getline(iss, var, ',')) {
            while (var[0] == ' '){
              var.erase(0, 1);
            }
            int space = var.find(" ");
            std::string type = var.substr(0, space);
            std::string var_name = var.substr(space+1);
            var_name.erase(std::remove_if(var_name.begin(), var_name.end(),
                           [](char c){
                             return std::isspace(static_cast<unsigned char>(c));
                           }), var_name.end());
            IR::Variable_type var_type = IR::str_to_variable_type(type);

            Variable* v = new Variable(var_name, var_type);
            // cout << "created v with name: "  << v->name << "\n";
            // cout << "created v with type: "  << type << "\n";
            f->arguments.push_back(v);
          };
        }
      }
  };

  template<> struct action < Args_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      std::string arg_str = in.string();
      // cout << "in args rule\n";
      arg_str.erase(std::remove_if(arg_str.begin(), arg_str.end(),
                     [](char c){
                       return std::isspace(static_cast<unsigned char>(c));
                     }), arg_str.end());
      if (arg_str.length() > 0){
        std::vector<Item*> args;
        std::vector<std::string> args_str;
        size_t index = 0;
        while ((index = arg_str.find(',')) != std::string::npos) {
          args_str.push_back(arg_str.substr(0, index));
          arg_str.erase(0, index+1);
        }
        args_str.push_back(arg_str);
        for (std::string a : args_str){
          if (IR::is_int(a)){
            Number* n = new Number(std::stoll(a));
            // cout << "parsed number " << a << "\n";
            args.push_back(n);
          } else {
            Variable* v = new Variable(a);
            // cout << "parsed var " << v->name << "\n";
            args.push_back(v);
          }
        }
        parsed_args.push_back(args);
      } else {
        parsed_args.push_back({});
      }
    }
  };

  template<> struct action < Instruction_definition_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      // cout << "in instruction definition\n";
      auto i = new Instruction_definition();
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.back())) {
        i->var = v;
      }
      i->type = parsed_variable_types.back();

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_assign_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      // cout << "in instruction assign\n";
      auto i = new Instruction_assign();
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size()-2))){
        i->dest = v;
      }
      i->source = parsed_items.back();

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_op_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      // cout << "in instruction op\n";
      auto i = new Instruction_op();
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size()-3))){
        i->dest = v;
      }
      i->t1 = parsed_items.at(parsed_items.size() - 2);
      i->t2 = parsed_items.back();
      i->op = parsed_operations.back();

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_load_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Instruction_load();
      // cout << "in instruction load\n";

      auto a = parsed_args.back();
      // cout << "dest: " <<parsed_items.at(parsed_items.size() - num_indices - 2)->to_string() << "\n";
      // cout << "source: " <<parsed_items.at(parsed_items.size() - num_indices - 1)->to_string() << "\n";

      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size() - 2))){
        i->dest = v;
      }
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.back())){
        i->source = v;
      }
      i->indices = a;

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_store_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Instruction_store();
      // cout << "in instruction store\n";
      auto a = parsed_args.back();

      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size() - 2))){
        i->dest = v;
      }
      i->source = parsed_items.back();
      i->indices = a;

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_length_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Instruction_length();
      // cout << "in instruction length\n";

      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size() - 3))){
        i->dest = v;
      }
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size() - 2))){
        i->source = v;
      }
      i->dimension = parsed_items.back();

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_array_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      // cout << "parsing new array instruction\n";
      auto i = new Instruction_array();

      if (Variable* v = dynamic_cast<Variable*>(parsed_items.back())) {
        i->dest = v;
      }
      i->args = parsed_args.back();

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_goto_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Instruction_goto();
      // cout << "in instruction goto\n";
      if (Label* l = dynamic_cast<Label*>(parsed_items.back())){
        i->label = l;
      }

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_label_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Instruction_label();
      // cout << "in instruction label\n";
      if (Label* l = dynamic_cast<Label*>(parsed_items.back())){
        i->label = l;
      }

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_jump_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Instruction_jump();
      // cout << "in instruction jump\n";

      i->check = parsed_items.at(parsed_items.size() -3);

      if (Label* l = dynamic_cast<Label*>(parsed_items.at(parsed_items.size() - 2))){
        i->label1 = l;
      }
      if (Label* l = dynamic_cast<Label*>(parsed_items.back())){
        i->label2 = l;
      }

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_ret_void_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Instruction_ret_void();
      // cout << "in instruction retvoid\n";
      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_ret_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Instruction_ret();
      // cout << "in instruction ret\n";
      i->t = parsed_items.back();
      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_call_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Instruction_call();
      // cout << "in instruction call\n";
      i->callee = parsed_items.back();
      i->args = parsed_args.back();
      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_call_store_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Instruction_call_store();
      // cout << "in instruction call store\n";
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size()-2))){
        // cout << "created dest\n";
        i->dest = v;
      }
      i->callee = parsed_items.back();
      i->args = parsed_args.back();

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  Program parse_file (char *fileName){

    /*
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< Program_rule >();

    /*
     * Parse.
     */
    file_input< > fileInput(fileName);
    Program p;
    parse< Program_rule, action >(fileInput, p);

    return p;
  }
}
