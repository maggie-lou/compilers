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

#include <L3.h>
#include <parser.h>
#include <utils.h>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace L3 {

  /*
   * Data required to parse
   */
  std::vector<Item*> parsed_items;
  std::vector<std::string> parsed_operations;
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

  struct Vars_rule:
    pegtl::seq<
      pegtl::star<
        var,
        seps
      >,
      pegtl::star<
        seps,
        pegtl::one<','>,
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
      pegtl::string<'>','>'>
    > {};

  struct Cmp_rule:
    pegtl::sor<
      pegtl::string<'<','='>,
      pegtl::string<'>','='>,
      pegtl::one<'<'>,
      pegtl::one<'>'>,
      pegtl::one<'='>
    > {};

  struct U_rule:
    pegtl::sor<
      Var_rule,
      Label_rule
    > {};

  struct Callee_other_rule:
    pegtl::sor<
      pegtl::string<'p','r','i','n','t'>,
      pegtl::string<'a','l','l','o','c','a','t','e'>,
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

  struct Instruction_cmp_rule:
    pegtl::seq<
      Var_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      T_rule,
      seps,
      Cmp_rule,
      seps,
      T_rule
    > {};

  struct Instruction_load_rule:
    pegtl::seq<
      Var_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      pegtl::string<'l','o','a','d'>,
      seps,
      Var_rule
    > {};

  struct Instruction_store_rule:
    pegtl::seq<
      pegtl::string<'s','t','o','r','e'>,
      seps,
      Var_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      S_rule
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
      Var_rule,
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
      pegtl::seq< pegtl::at<Instruction_op_rule>, Instruction_op_rule >,
      pegtl::seq< pegtl::at<Instruction_cmp_rule>, Instruction_cmp_rule >,
      pegtl::seq< pegtl::at<Instruction_assign_rule>, Instruction_assign_rule >,
      pegtl::seq< pegtl::at<Instruction_jump_rule>, Instruction_jump_rule >,
      pegtl::seq< pegtl::at<Instruction_goto_rule>, Instruction_goto_rule >,
      pegtl::seq< pegtl::at<Instruction_label_rule>, Instruction_label_rule >,
      pegtl::seq< pegtl::at<Instruction_ret_rule>, Instruction_ret_rule >,
      pegtl::seq< pegtl::at<Instruction_ret_void_rule>, Instruction_ret_void_rule >,
      pegtl::seq< pegtl::at<Instruction_call_store_rule>, Instruction_call_store_rule >,
      pegtl::seq< pegtl::at<Instruction_call_rule>, Instruction_call_rule >
    > { };

  struct Instructions_rule:
    pegtl::plus<
      pegtl::seq<
        seps,
        Instruction_rule,
        seps
      >
    > { };

  struct Function_rule:
    pegtl::seq<
      pegtl::string<'d','e','f','i','n','e'>,
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
      Instructions_rule,
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
      cout << "label " << in.string();
      auto i = new Label();
      i->name = in.string();
      parsed_items.push_back(i);
      Function* f = p.functions.back();
      if (p.longest_label_name.length() < in.string().length()){
        p.longest_label_name = in.string();
      }
      f->labels.push_back(i);
    }
  };

  template<> struct action < Number_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Number();
      i->n = std::stoll(in.string());
      cout << "number " << i->n << endl;
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Var_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      cout << "Variable " << in.string() << endl;
      auto i = new Variable();
      i->name = in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Op_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      parsed_operations.push_back(in.string());
    }
  };

  template<> struct action < Cmp_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      parsed_operations.push_back(in.string());
    }
  };

  template<> struct action < Callee_other_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "Callee other " << endl;
      auto i = new Sys_call();
      i->name = in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Function_name_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto f = new Function();
      f->name = in.string();
      if (p.longest_label_name.length() < in.string().length()){
        p.longest_label_name = in.string();
      }
      p.function_names.push_back(in.string());
      p.functions.push_back(f);
    }
  };

  template<> struct action < Vars_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      // cout << "in parser vars rule\noriginal vars string: " << in.string() << "with length " << in.string().length() << "\n";
      std::string vars_str = in.string();
      vars_str.erase(std::remove_if(vars_str.begin(), vars_str.end(),
                     [](char c){
                       return std::isspace(static_cast<unsigned char>(c));
                     }), vars_str.end());
      // cout << "after removing spaces: " << vars_str << "with length " << vars_str.length() <<"\n";
      if (vars_str.length() > 0){
        Function* f = p.functions.back();
        size_t index = 0;
        while ((index = vars_str.find(',')) != std::string::npos) {
          Variable* v = new Variable(vars_str.substr(0, index));
          cout << "created var with name: " << v->name << "\n";
          f->arguments.push_back(v);
          vars_str.erase(0, index+1);
        }
        Variable* v = new Variable(vars_str);
        cout << "created var with name: " << v->name << "\n";
        f->arguments.push_back(v);
      }
    }
  };

  template<> struct action < Args_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "Args rule" << endl;
      std::string arg_str = in.string();
      arg_str.erase(std::remove_if(arg_str.begin(), arg_str.end(),
                     [](char c){
                       return std::isspace(static_cast<unsigned char>(c));
                     }), arg_str.end());
      std::vector<Item*> args;
      std::vector<std::string> args_str;
      size_t index = 0;
      while ((index = arg_str.find(',')) != std::string::npos) {
        args_str.push_back(arg_str.substr(0, index));
        arg_str.erase(0, index+1);
      }
      args_str.push_back(arg_str);
      for (std::string a : args_str){
        cout << "Arg ";
        if (L3::is_int(a)){
          Number* n = new Number(std::stoll(a));
          cout << to_string(n->n)<< endl;
          args.push_back(n);
        } else {
          Variable* v = new Variable(a);
          cout << v->name << endl;
          args.push_back(v);
        }
      }
      parsed_args.push_back(args);
    }
  };

  template<> struct action < Instruction_assign_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "assignment ";
      auto i = new Instruction_assign();
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size()-2))){
        cout << "with variable name " << v->name << endl;
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
    cout << "assignment op ";
      auto i = new Instruction_op();
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size()-3))){
        i->dest = v;
      }
      i->t1 = parsed_items.at(parsed_items.size() - 2);
      i->t2 = parsed_items.back();
      i->op = parsed_operations.back();

      cout << i->op << endl;

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_cmp_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      cout << "Cmp" << endl;
      auto i = new Instruction_cmp();
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size()-3))){
        i->dest = v;
      }
      i->t1 = parsed_items.at(parsed_items.size() - 2);
      i->t2 = parsed_items.back();
      i->cmp = parsed_operations.back();

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_load_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      cout << "Load " << endl;
      auto i = new Instruction_load();
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size()-2))){
        i->dest = v;
      }
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.back())){
        i->source = v;
      }

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_store_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      cout << "Store" << endl;
      auto i = new Instruction_store();
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size()-2))){
        i->dest = v;
      }
      i->source = parsed_items.back();

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_goto_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      cout << "Goto" << endl;
      auto i = new Instruction_goto();
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
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size()-2))){
        i->var = v;
      }
      if (Label* l = dynamic_cast<Label*>(parsed_items.back())){
        i->label = l;
      }

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_ret_void_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Instruction_ret_void();
      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_ret_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Instruction_ret();
      i->t = parsed_items.back();
      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_call_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Instruction_call();
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
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size()-2))){
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
