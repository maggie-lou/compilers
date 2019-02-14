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

#include <iostream>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include <L3.h>
#include <parser.h>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;

namespace L3 {

  /*
   * Data required to parse
   */
  std::vector<Item*> parsed_items;
  std::vector<std::string> parsed_operations;


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

  struct Var_rule:
    name {};

  struct Vars_rule:
    pegtl::seq<
      Var_rule,
      seps,
      pegtl::star<
        seps,
        pegtl::one<','>,
        seps,
        Var_rule
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

  struct Instruction_s_rule:
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

  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<Instruction_load_rule>, Instruction_load_rule >,
      pegtl::seq< pegtl::at<Instruction_store_rule>, Instruction_store_rule >,
      pegtl::seq< pegtl::at<Instruction_op_rule>, Instruction_op_rule >,
      pegtl::seq< pegtl::at<Instruction_cmp_rule>, Instruction_cmp_rule >,
      pegtl::seq< pegtl::at<Instruction_s_rule>, Instruction_s_rule >,
      pegtl::seq< pegtl::at<Instruction_goto_rule>, Instruction_goto_rule >
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
      Label_rule,
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
      pegtl::one<'}'>,
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

  template<> struct action < Var_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
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

  template<> struct action < Instruction_s_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Instruction_s();
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

  template<> struct action < Instruction_cmp_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
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
      auto i = new Instruction_goto();
      if (Label* l = dynamic_cast<Label*>(parsed_items.back())){
        i->label = l;
      }

      Function* f = p.functions.back();
      f->instructions.push_back(i);
    }
  };

  Program parse_file (char *fileName){

    /*
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< grammar >();

    /*
     * Parse.
     */
    file_input< > fileInput(fileName);
    Program p;
    parse< grammar, action >(fileInput, p);

    return p;
  }
}
