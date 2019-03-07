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
#include <stack>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include <LB.h>
#include <parser.h>
#include <utils.h>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace LB {

  /*
   * Data required to parse
   */
  vector<Item*> parsed_items;
  vector<std::string> parsed_operations;
  vector<VariableType> parsed_variable_types;
  vector<vector<Item*>> parsed_args;
  vector<vector<Variable*>> parsed_def_vars;
  stack<Scope*> scopes;

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

  struct Label_rule:
    label {};

  struct Number_rule:
    number {};

  struct Name_rule:
    name {};

  struct Function_name_rule:
    name {};

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

  struct T_rule:
    pegtl::sor<
      Name_rule,
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

  struct Args_t_rule:
    pegtl::sor<
      name,
      number
    > {};

  struct Args_rule:
    pegtl::seq<
      pegtl::star<
        Args_t_rule,
        pegtl::star<
          pegtl::one<','>,
          seps,
          Args_t_rule
        >
      >
    > {};

  struct Names_rule:
    pegtl::seq<
      pegtl::star<
        name,
        pegtl::star<
          pegtl::one<','>,
          seps,
          name
        >
      >
    > {};

  struct Function_arg_name_rule:
    name {};

  struct Function_arg_rule:
    pegtl::seq<
      Type_rule,
      seps,
      Function_arg_name_rule
    > {};

  struct Function_args_rule:
    pegtl::seq<
      pegtl::star<
        Function_arg_rule,
        pegtl::star<
          seps,
          pegtl::one<','>,
          seps,
          Function_arg_rule
        >
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
      Names_rule
    > {};

  struct Instruction_assign_rule:
    pegtl::seq<
      Name_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      S_rule
    > {};

  struct Cond_rule:
    pegtl::seq<
      T_rule,
      seps,
      Op_rule,
      seps,
      T_rule
    > {};

  struct Instruction_op_rule:
    pegtl::seq<
      Name_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      Cond_rule
    > {};

  struct Instruction_if_rule:
    pegtl::seq<
      pegtl::string<'i','f'>,
      seps,
      pegtl::string<'('>,
      seps,
      Cond_rule,
      seps,
      pegtl::string<')'>,
      seps,
      Label_rule,
      seps,
      Label_rule
    > {};

  struct Instruction_while_rule:
    pegtl::seq<
      pegtl::string<'w','h','i','l','e'>,
      seps,
      pegtl::string<'('>,
      seps,
      Cond_rule,
      seps,
      pegtl::string<')'>,
      seps,
      Label_rule,
      seps,
      Label_rule
    > {};

  struct Instruction_continue_rule:
    pegtl::string<'c','o','n','t','i','n','u','e'> {};

  struct Instruction_break_rule:
    pegtl::string<'b','r','e','a','k'> {};

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
      Name_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      Name_rule,
      Array_accesses_rule
    > {};

  struct Instruction_store_rule:
    pegtl::seq<
      Name_rule,
      Array_accesses_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      S_rule
    > {};

  struct Instruction_length_rule:
    pegtl::seq<
      Name_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      pegtl::string<'l','e','n','g','t','h'>,
      seps,
      Name_rule,
      seps,
      T_rule
    > {};

  struct Instruction_array_rule:
    pegtl::seq<
      Name_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      pegtl::string<'n','e','w'>,
      seps,
      pegtl::string<'A','r','r','a','y'>,
      seps,
      pegtl::one<'('>,
      seps,
      Args_rule,
      seps,
      pegtl::one<')'>
    > {};

  struct Instruction_tuple_rule:
    pegtl::seq<
      Name_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      pegtl::string<'n','e','w'>,
      seps,
      pegtl::string<'T','u','p','l','e'>,
      seps,
      pegtl::one<'('>,
      seps,
      T_rule,
      seps,
      pegtl::one<')'>
    > {};

  struct Instruction_label_rule:
    pegtl::seq<
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
      Name_rule,
      seps,
      pegtl::one<'('>,
      seps,
      Args_rule,
      seps,
      pegtl::one<')'>
    > {};

  struct Instruction_call_store_rule:
    pegtl::seq<
      Name_rule,
      seps,
      pegtl::string<'<','-'>,
      seps,
      Name_rule,
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

  struct Instruction_print_rule:
    pegtl::seq<
      pegtl::string<'p','r','i','n','t'>,
      seps,
      pegtl::one<'('>,
      seps,
      T_rule,
      seps,
      pegtl::one<')'>
    > {};

  struct Scope_start_rule:
    pegtl::one<'{'> {};

  struct Scope_end_rule:
    pegtl::one<'}'> {};

  struct Instruction_rule;

  struct Scope_rule:
    pegtl::seq<
      Scope_start_rule,
      seps,
      pegtl::plus<
        seps,
        Instruction_rule,
        seps
      >,
      seps,
      Scope_end_rule
    > {};

  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<Instruction_if_rule>, Instruction_if_rule >,
      pegtl::seq< pegtl::at<Instruction_while_rule>, Instruction_while_rule >,
      pegtl::seq< pegtl::at<Instruction_continue_rule>, Instruction_continue_rule >,
      pegtl::seq< pegtl::at<Instruction_break_rule>, Instruction_break_rule >,
      pegtl::seq< pegtl::at<Instruction_load_rule>, Instruction_load_rule >,
      pegtl::seq< pegtl::at<Instruction_store_rule>, Instruction_store_rule >,
      pegtl::seq< pegtl::at<Instruction_print_rule>, Instruction_print_rule >,
      pegtl::seq< pegtl::at<Instruction_array_rule>, Instruction_array_rule >,
      pegtl::seq< pegtl::at<Instruction_tuple_rule>, Instruction_tuple_rule >,
      pegtl::seq< pegtl::at<Instruction_call_store_rule>, Instruction_call_store_rule >,
      pegtl::seq< pegtl::at<Instruction_call_rule>, Instruction_call_rule >,
      pegtl::seq< pegtl::at<Instruction_definition_rule>, Instruction_definition_rule >,
      pegtl::seq< pegtl::at<Instruction_op_rule>, Instruction_op_rule >,
      pegtl::seq< pegtl::at<Instruction_length_rule>, Instruction_length_rule >,
      pegtl::seq< pegtl::at<Instruction_assign_rule>, Instruction_assign_rule >,
      pegtl::seq< pegtl::at<Instruction_goto_rule>, Instruction_goto_rule >,
      pegtl::seq< pegtl::at<Instruction_ret_rule>, Instruction_ret_rule >,
      pegtl::seq< pegtl::at<Instruction_ret_void_rule>, Instruction_ret_void_rule >,
      pegtl::seq< pegtl::at<Instruction_label_rule>, Instruction_label_rule >,
      Scope_rule
    > { };

  struct Function_rule:
    pegtl::seq<
      Function_type_rule,
      seps,
      Function_name_rule,
      seps,
      pegtl::one<'('>,
      seps,
      Function_args_rule,
      seps,
      pegtl::one<')'>,
      seps,
      Scope_rule
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
      auto i = new Label();
      i->name = in.string();
      parsed_items.push_back(i);
      if (p.longest_label.length() < in.string().length()){
        p.longest_label = in.string();
      }
    }
  };

  template<> struct action < Number_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Number();
      i->n = std::stoll(in.string());
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Name_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Variable();
      i->name = in.string();
      i->scope = scopes.top();
      parsed_items.push_back(i);
      if (p.longest_var.length() < in.string().length()){
        p.longest_var = in.string();
      }
    }
  };

  template<> struct action < Function_arg_name_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Variable();
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
      parsed_operations.push_back(in.string());
    }
  };

  template<> struct action < Type_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      VariableType type;
      type.name = in.string();
      parsed_variable_types.push_back(type);
    }
  };

  template<> struct action < Function_type_rule > {
    template< typename Input >
      static void apply( const Input & in, Program & p){
        auto f = new Function();
        VariableType type(in.string());
        f->type = type;
        p.functions.push_back(f);
      }
  };

  template<> struct action < Function_name_rule > {
    template< typename Input >
      static void apply( const Input & in, Program & p){
        Function* f = p.functions.back();
        f->name = in.string();
        if (p.longest_label.length() < in.string().length()){
          p.longest_label = ":" + in.string();
        }
      }
  };

  template<> struct action < Function_arg_rule > {
    template< typename Input >
      static void apply( const Input & in, Program & p){
        Function* f = p.functions.back();
        VariableType var_type = parsed_variable_types.back();
        std::string var_name = parsed_items.back()->to_string();

        Variable* v = new Variable(var_name, var_type);
        f->arguments.push_back(v);
      }
  };

  template<> struct action < Array_accesses_rule > {
    template< typename Input >
      static void apply( const Input & in, Program & p){
        std::vector<Item*> array_accesses;
        std::string accesses_str = in.string();
        std::vector<std::string> accesses_strs;
        size_t index = 0;
        while ((index = accesses_str.find(']')) != std::string::npos) {
          accesses_strs.push_back(accesses_str.substr(0, index));
          accesses_str.erase(0, index+1);
        }
        for (std::string a : accesses_strs){
          a = a.substr(1);
          if (LB::is_int(a)){
            Number* n = new Number(std::stoll(a));
            array_accesses.push_back(n);
          } else {
            Variable* v = new Variable(a);
            v->scope = scopes.top();
            array_accesses.push_back(v);
          }
        }
        parsed_args.push_back(array_accesses);
      }
  };

  template<> struct action < Args_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      std::string arg_str = in.string();
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
          if (LB::is_int(a)){
            Number* n = new Number(std::stoll(a));
            args.push_back(n);
          } else {
            Variable* v = new Variable(a);
            v->scope = scopes.top();
            args.push_back(v);
          }
        }
        parsed_args.push_back(args);
      } else {
        parsed_args.push_back({});
      }
    }
  };

  template<> struct action < Names_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      std::string name_str = in.string();
      name_str.erase(std::remove_if(name_str.begin(), name_str.end(),
                     [](char c){
                       return std::isspace(static_cast<unsigned char>(c));
                     }), name_str.end());
      if (name_str.length() > 0){
        vector<Variable*> names;
        vector<std::string> names_str;
        size_t index = 0;
        while ((index = name_str.find(',')) != std::string::npos) {
          names_str.push_back(name_str.substr(0, index));
          name_str.erase(0, index+1);
        }
        names_str.push_back(name_str);
        for (std::string a : names_str){
          Variable* v = new Variable(a);
          // cout << "in names rule, scope size: " << scopes.size() << endl;
          v->scope = scopes.top();
          names.push_back(v);
        }
        parsed_def_vars.push_back(names);
      } else {
        parsed_def_vars.push_back({});
      }
    }
  };

  template<> struct action < Instruction_definition_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "in definition\n";
      Function* f = p.functions.back();
      auto i = new Instruction_definition();
      i->vars = parsed_def_vars.back();
      i->type = parsed_variable_types.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_assign_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "in assign\n";
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
    cout << "in op\n";
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

  template<> struct action < Instruction_if_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "in if\n";
      auto i = new Instruction_if();
      if (Label* l = dynamic_cast<Label*>(parsed_items.back())){
        i->label2 = l;
      }
      if (Label* l = dynamic_cast<Label*>(parsed_items.at(parsed_items.size() - 2))){
        i->label1 = l;
      }
      i->t1 = parsed_items.at(parsed_items.size() - 4);
      i->t2 = parsed_items.at(parsed_items.size() - 3);
      i->op = parsed_operations.back();

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_while_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "in while\n";
      auto i = new Instruction_while();
      if (Label* l = dynamic_cast<Label*>(parsed_items.back())){
        i->label2 = l;
      }
      if (Label* l = dynamic_cast<Label*>(parsed_items.at(parsed_items.size() - 2))){
        i->label1 = l;
      }
      i->t1 = parsed_items.at(parsed_items.size() - 4);
      i->t2 = parsed_items.at(parsed_items.size() - 3);
      i->op = parsed_operations.back();

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_continue_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "in continue\n";
      auto i = new Instruction_continue();
      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_break_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "in break\n";
      auto i = new Instruction_break();
      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_load_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "in load\n";
      auto i = new Instruction_load();
      auto a = parsed_args.back();

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
    cout << "in store\n";
      auto i = new Instruction_store();
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
    cout << "in array\n";
      auto i = new Instruction_array();

      if (Variable* v = dynamic_cast<Variable*>(parsed_items.back())) {
        i->dest = v;
      }
      i->dimensions = parsed_args.back();
      i->is_tuple = false;

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_tuple_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "in tuple\n";
      auto i = new Instruction_array();

      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size()-2))) {
        i->dest = v;
      }
      vector<Item*> dimensions;
      dimensions.push_back(parsed_items.back());
      i->dimensions = dimensions;
      i->is_tuple = true;

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_label_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "in label\n";
      auto i = new Instruction_label();
      if (Label* l = dynamic_cast<Label*>(parsed_items.back())){
        i->label = l;
      }

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_goto_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Instruction_goto();
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
    cout << "in ret void\n";
      auto i = new Instruction_ret_void();
      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_ret_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "in ret\n";
      auto i = new Instruction_ret();
      i->t = parsed_items.back();
      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_call_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "in call\n";
      auto i = new Instruction_call();
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.back())){
        i->callee = v;
      }
      i->args = parsed_args.back();
      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_call_store_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "in call store\n";
      auto i = new Instruction_call_store();
      if (Variable* v = dynamic_cast<Variable*>(parsed_items.at(parsed_items.size()-2))){
        i->dest = v;
      }
      if (Variable* v2 = dynamic_cast<Variable*>(parsed_items.back())){
        i->callee = v2;
      }
      i->args = parsed_args.back();

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_print_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "in print\n";
      auto i = new Instruction_print();
      i->t = parsed_items.back();
      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  template<> struct action < Scope_start_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "in scope start\n";
      auto s = new Scope();
      if (!scopes.empty()){
        Scope* parent = scopes.top();
        s->parent = parent;
        parent->children.push_back(s);
      }
      scopes.push(s);
    }
  };

  template<> struct action < Scope_end_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
    cout << "in scope end\n";
    cout << "scope size before pop: " << scopes.size() << endl;
      scopes.pop();
    cout << "scope size after pop: " << scopes.size() << endl;
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
