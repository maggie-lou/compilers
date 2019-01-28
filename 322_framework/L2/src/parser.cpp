
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

#include <L2.h>
#include <parser.h>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace L2 {

  /*
   * Data required to parse
   */
  std::vector<Item> parsed_destinations;
  std::vector<std::string> parsed_operations;
  std::vector<Item> parsed_sources;
  std::vector<Item> parsed_items;
  std::vector<Comparison> parsed_comparisons;

  /*
   * Grammar rules from now on.
   */
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

  struct function_name:
    label {};

  struct argument_number:
    number {};

  struct local_number:
    number {} ;

  struct comment:
    pegtl::disable<
      TAOCPP_PEGTL_STRING( "//" ),
      pegtl::until< pegtl::eolf >
    > {};

  /*
   * Keywords.
   */
  struct str_return : TAOCPP_PEGTL_STRING( "return" ) {};

  struct seps:
    pegtl::star<
      pegtl::sor<
        pegtl::ascii::space,
        comment
      >
    > {};

  struct Label_rule:
    label {};

  struct Number_rule:
    number {};

  struct Var_rule:
    pegtl::seq<
      pegtl::one< '%' >,
      name
    > { };

  struct E_rule:
    pegtl::seq<
      Number_rule
    > { };

  struct Cmp_rule:
    pegtl::sor<
      pegtl::string<'<','='>,
      pegtl::string<'<'>,
      pegtl::string<'='>
    > { };

  struct Sop_rule:
    pegtl::sor<
      pegtl::string<'<','<','='>,
      pegtl::string<'>','>','='>
    > { };

  struct Aop_rule:
    pegtl::sor<
      pegtl::string<'+','='>,
      pegtl::string<'-','='>,
      pegtl::string<'*','='>,
      pegtl::string<'&','='>
    > { };

  struct Assign_rule:
    pegtl::string<'<','-'> { };

  struct Instruction_return_rule:
    pegtl::seq<
      str_return
    > { };

  struct Sx_rule:
    pegtl::sor<
      pegtl::string<'r','c','x'>,
      Var_rule
    > { };

  struct A_other_rule:
    pegtl::sor<
      pegtl::string<'r','d','i'>,
      pegtl::string<'r','d','x'>,
      pegtl::string<'r','8'>,
      pegtl::string<'r','9'>,
      pegtl::string<'r','s','i'>
    > {};

  struct A_rule:
    pegtl::sor<
      Sx_rule,
      A_other_rule
    > {};

  struct W_other_rule:
    pegtl::sor<
      pegtl::string<'r','b','x'>,
      pegtl::string<'r','a','x'>,
      pegtl::string<'r','b','p'>,
      pegtl::string<'r','1','0'>,
      pegtl::string<'r','1','1'>,
      pegtl::string<'r','1','2'>,
      pegtl::string<'r','1','3'>,
      pegtl::string<'r','1','4'>,
      pegtl::string<'r','1','5'>
    > {};

  struct W_rule:
    pegtl::sor<
      W_other_rule,
      A_rule
    > {};

  struct X_other_rule:
    pegtl::string<'r','s','p'> {};

  struct X_rule:
    pegtl::sor<
      W_rule,
      X_other_rule
    > {};

  struct T_rule :
    pegtl::sor<
      Number_rule,
      X_rule
    > {};

  struct S_rule :
    pegtl::sor<
      T_rule,
      Label_rule
    > {};

  struct M_rule:
    pegtl::seq<
      Number_rule
    > { };

  struct Address_rule:
    pegtl::seq<
      pegtl::string<'m','e','m'>,
      seps,
      X_rule,
      seps,
      M_rule
    > { };

  // t cmp t
  struct Comparison_rule:
    pegtl::seq<
      T_rule,
      seps,
      Cmp_rule,
      seps,
      T_rule
    > { };

  struct Source_rule:
    pegtl::sor<
      S_rule,
      Address_rule,
      Comparison_rule,
      M_rule
    > { };

  struct Destination_rule:
    pegtl::sor<
      W_rule,
      Address_rule
    > { };

  struct Stack_arg_rule:
    pegtl::seq<
      pegtl::string<'<','-'>,
      seps,
      pegtl::string<'s','t','a','c','k','-','a','r','g'>
    > { };

  struct Operation_rule:
    pegtl::sor<
      Stack_arg_rule,
      Assign_rule,
      Aop_rule,
      Sop_rule
    > { };

  struct Assignment_cmp_rule:
    pegtl::seq<
      Destination_rule,
      seps,
      Assign_rule,
      seps,
      Comparison_rule
    > { };

  struct Assignment_rule:
    pegtl::seq<
      Destination_rule,
      seps,
      Operation_rule,
      seps,
      Source_rule
    > { };

  // cjump t cmp t label label
  struct Cjump_rule:
    pegtl::seq<
      pegtl::string<'c','j','u','m','p'>,
      seps,
      Comparison_rule,
      seps,
      Label_rule,
      seps,
      Label_rule
    > { };

  // cjump t cmp t label
  struct Cjump_fallthrough_rule:
    pegtl::seq<
      pegtl::string<'c','j','u','m','p'>,
      seps,
      Comparison_rule,
      seps,
      Label_rule
    > { };

  // label
  struct Label_instruction_rule:
    pegtl::seq<
      Label_rule
    > { };

  // goto label
  struct Goto_rule:
    pegtl::seq<
      pegtl::string<'g','o','t','o'>,
      seps,
      Label_rule
    > { };

  struct U_rule:
    pegtl::sor<
      W_rule,
      Label_rule
    > { };

  // call u N
  struct Custom_func_call_rule:
    pegtl::seq<
      pegtl::string<'c','a','l','l'>,
      seps,
      U_rule,
      seps,
      Number_rule
    > { };

  struct System_func_rule:
    pegtl::sor<
      pegtl::string<'p','r','i','n','t'>,
      pegtl::string<'a','l','l','o','c','a','t','e'>,
      pegtl::string<'a','r','r','a','y','-','e','r','r','o','r'>
    > { };

  // call print 1 | call allocate 2 | call array-error 2
  struct System_func_call_rule:
    pegtl::seq<
      pegtl::string<'c','a','l','l'>,
      seps,
      System_func_rule,
      seps,
      number
    > { };

  // ++, --
  struct Inc_or_dec_rule:
    pegtl::sor<
      pegtl::string<'+','+'>,
      pegtl::string<'-','-'>
    > { };

  // w++, w--
  struct Inc_or_dec_instruction_rule:
    pegtl::seq<
      W_rule,
      seps,
      Inc_or_dec_rule
    > { };

  // w @ w w E
  struct At_arithmetic_rule:
    pegtl::seq<
      W_rule,
      seps,
      pegtl::one<'@'>,
      seps,
      W_rule,
      seps,
      W_rule,
      seps,
      E_rule
    > { };

  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<Instruction_return_rule>            , Instruction_return_rule             >,
      pegtl::seq< pegtl::at<Assignment_cmp_rule>            , Assignment_cmp_rule             >,
      pegtl::seq< pegtl::at<Assignment_rule>            , Assignment_rule             >,
      pegtl::seq< pegtl::at<Label_instruction_rule>            , Label_instruction_rule             >,
      pegtl::seq< pegtl::at<Cjump_rule>            , Cjump_rule             >,
      pegtl::seq< pegtl::at<Cjump_fallthrough_rule>            , Cjump_fallthrough_rule             >,
      pegtl::seq< pegtl::at<Goto_rule>            , Goto_rule             >,
      pegtl::seq< pegtl::at<Custom_func_call_rule>            , Custom_func_call_rule             >,
      pegtl::seq< pegtl::at<System_func_call_rule>            , System_func_call_rule             >,
      pegtl::seq< pegtl::at<Inc_or_dec_instruction_rule>            , Inc_or_dec_instruction_rule             >,
      pegtl::seq< pegtl::at<At_arithmetic_rule>            , At_arithmetic_rule             >
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
      pegtl::one< '(' >,
      seps,
      function_name,
      seps,
      argument_number,
      seps,
      local_number,
      seps,
      Instructions_rule,
      seps,
      pegtl::one< ')' >
    > {};

  struct Functions_rule:
    pegtl::plus<
      seps,
      Function_rule,
      seps
    > {};

  struct entry_point_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      seps,
      label,
      seps,
      Functions_rule,
      seps,
      pegtl::one< ')' >,
      seps
    > { };

  struct grammar :
    pegtl::must<
      entry_point_rule
    > {};



  /*
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < label > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      if (p.entryPointLabel.empty()){
        p.entryPointLabel = "_" + in.string().substr(1);
      } else {
        abort();
      }
    }
  };

  template<> struct action < function_name > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto newF = new Function();
      newF->name = "_" + in.string().substr(1);
      p.functions.push_back(newF);
    }
  };

  template<> struct action < argument_number > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->arguments = std::stoll(in.string());
    }
  };

  template<> struct action < local_number > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->locals = std::stoll(in.string());
    }
  };

  template<> struct action < str_return > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_ret();
      i->locals = currentF->locals;
      i->arguments = currentF->arguments;
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Label_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      Item i;
      i.value = "_" + in.string().substr(1);
      i.is_address = false;
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Number_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      Item i;
      i.is_address = false;
      i.value = "$" + in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Var_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      Item i;
      i.value = in.string().substr(1);
      i.is_address = false;
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Sx_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      Item i;
      i.is_address = false;
      i.value = "%" + in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < A_other_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      Item i;
      i.is_address = false;
      i.value = "%" + in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < W_other_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      Item i;
      i.is_address = false;
      i.value = "%" + in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < X_other_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      Item i;
      i.is_address = false;
      i.value = "%" + in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Address_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto x = parsed_items.at(parsed_items.size() - 2);
      auto m = parsed_items.back();
      Address address;
      address.offset = m.value;
      address.r = x.value;

      Item i;
      i.is_address = true;
      i.address = address;
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Comparison_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      Comparison comparison;
      comparison.left = parsed_items.at(parsed_items.size() - 3).value;
      comparison.cmp_sign = parsed_items.at(parsed_items.size() - 2).value;
      comparison.right = parsed_items.back().value;
      parsed_comparisons.push_back(comparison);
    }
  };

  template<> struct action < Assignment_cmp_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto a = new AssignmentCmp();
      auto d = parsed_destinations.back();
      auto s = parsed_comparisons.back();

      a->d = d;
      a->s = s;
      auto currentF = p.functions.back();
      currentF->instructions.push_back(a);
    }
  };

  template<> struct action < Cjump_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto cjump = new Cjump();
      auto c = parsed_comparisons.back();
      auto label1 = parsed_items.at(parsed_items.size()-2);
      auto label2 = parsed_items.back();

      cjump->c = c;
      cjump->label1 = label1.value;
      cjump->label2 = label2.value;
      auto currentF = p.functions.back();
      currentF->instructions.push_back(cjump);
    }
  };

  template<> struct action < Cjump_fallthrough_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto cjump = new Cjump_fallthrough();
      auto c = parsed_comparisons.back();
      auto label = parsed_items.back();

      cjump->c = c;
      cjump->label = label.value;
      auto currentF = p.functions.back();
      currentF->instructions.push_back(cjump);
    }
  };

  template<> struct action < Goto_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto g = new Goto();
      auto label = parsed_items.back();

      g->label = label.value;
      auto currentF = p.functions.back();
      currentF->instructions.push_back(g);
    }
  };

  template<> struct action < Custom_func_call_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto c = new Custom_func_call();
      auto u = parsed_items.at(parsed_items.size()-2);
      auto n = parsed_items.back();

      c->u = u.value;
      c->n = n.value;
      auto currentF = p.functions.back();
      currentF->instructions.push_back(c);
    }
  };

  template<> struct action < Label_instruction_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto l = new Label_instruction();
      auto label = parsed_items.back();

      l->label = label.value;
      auto currentF = p.functions.back();
      currentF->instructions.push_back(l);
    }
  };

  template<> struct action < System_func_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      Item i;
      i.is_address = false;
      i.value = in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Inc_or_dec_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      parsed_operations.push_back(in.string());
    }
  };

  template<> struct action < Inc_or_dec_instruction_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto i = new Inc_or_dec();
      auto d = parsed_items.back();
      auto op = parsed_operations.back();

      i->reg = d.value;
      i->op = op;
      auto currentF = p.functions.back();
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < At_arithmetic_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto at_a = new At_arithmetic();
      at_a->dest = (parsed_items.at(parsed_items.size() - 4)).value;

      at_a->r1 = (parsed_items.at(parsed_items.size() - 3)).value;
      at_a->r2 = (parsed_items.at(parsed_items.size() - 2)).value;
      at_a->n = (parsed_items.back()).value;

      auto currentF = p.functions.back();
      currentF->instructions.push_back(at_a);
    }
  };

  template<> struct action < System_func_call_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto s = new System_func_call();
      auto system_func = parsed_items.back();

      s->system_func = system_func.value;
      auto currentF = p.functions.back();
      currentF->instructions.push_back(s);
    }
  };

  template<> struct action < Assignment_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto a = new Assignment();
      auto d = parsed_destinations.back();
      auto op = parsed_operations.back();
      auto s = parsed_sources.back();

      a->d = d;
      a->op = op;
      a->s = s;
      auto currentF = p.functions.back();
      currentF->instructions.push_back(a);
    }
  };

  template<> struct action < Destination_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto dest = parsed_items.back();
      parsed_destinations.push_back(dest);
    }
  };

  template<> struct action < Source_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto source = parsed_items.back();
      parsed_sources.push_back(source);
    }
  };

  template<> struct action < Sop_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
    parsed_operations.push_back(in.string());
    }
  };

  template<> struct action < Aop_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
    parsed_operations.push_back(in.string());
    }
  };

  template<> struct action < Stack_arg_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
    parsed_operations.push_back("<- stack-arg");
    }
  };

  template<> struct action < Assign_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
    parsed_operations.push_back(in.string());
    }
  };

  template<> struct action < Cmp_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      Item i;
      i.is_address = false;
      i.value = in.string();
      parsed_items.push_back(i);
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

  Function parse_function (char *fileName){

    /*
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< grammar >();

    /*
     * Parse.
     */
    file_input< > fileInput(fileName);
    Function f;
    parse< grammar, action >(fileInput, f);

    return f;
  }

}
