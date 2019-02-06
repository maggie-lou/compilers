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
  std::vector<Item*> parsed_destinations;
  std::vector<std::string> parsed_operations;
  std::vector<Item*> parsed_sources;
  std::vector<Item*> parsed_items;
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

  struct Spill_var_rule:
    var {};

  struct Spill_prefix_rule:
    var {};

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

  struct Sx_other_rule:
    pegtl::string<'r', 'c', 'x'> {};

  struct Sx_rule:
    pegtl::sor<
      Sx_other_rule,
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
      X_rule,
      Number_rule
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
      Address_rule,
      Comparison_rule,
      S_rule,
      M_rule
    > { };

  struct Destination_rule:
    pegtl::sor<
      Address_rule,
      W_rule
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

  struct Spill_rule:
    pegtl::seq<
      Function_rule,
      seps,
      Spill_var_rule,
      seps,
      Spill_prefix_rule
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
      newF->name = in.string();
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
      auto i = new Label_item();
      i->label_name = in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Number_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto i = new Num_item();
      i->n = std::stoll(in.string());
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Var_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto i = new Var_item();
      i->var_name = in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Spill_var_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->var_name = in.string();
    }
  };

  template<> struct action < Spill_prefix_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->prefix = in.string();
    }
  };

  template<> struct action < Sx_other_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto i = new Register_item();
      i->register_name = in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < A_other_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Register_item();
      i->register_name = in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < W_other_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      auto i = new Register_item();
      i->register_name = in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < X_other_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto i = new Register_item();
      i->register_name = in.string();
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Address_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto i = new Address_item();
      if (Register_item* register_i = dynamic_cast<Register_item*>(parsed_items.at(parsed_items.size() - 2))){
        i->r = register_i->register_name;
      } else if (Var_item* var_i = dynamic_cast<Var_item*>(parsed_items.at(parsed_items.size() - 2))){
        i->r = var_i->var_name;
      }

      if (Num_item* num_i = dynamic_cast<Num_item*>(parsed_items.back())){
        i->offset = num_i->n;
      }
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Comparison_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      Comparison comparison;
      comparison.left = parsed_items.at(parsed_items.size() - 2);
      comparison.right = parsed_items.back();
      comparison.cmp_sign = parsed_operations.back();
      parsed_comparisons.push_back(comparison);
    }
  };

  template<> struct action < Assignment_cmp_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto a = new AssignmentCmp();
      a->d = parsed_destinations.back();
      a->c = parsed_comparisons.back();
      auto currentF = p.functions.back();
      currentF->instructions.push_back(a);
    }
  };

  template<> struct action < Cjump_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto cjump = new Cjump();
      cjump->c = parsed_comparisons.back();
      if (Label_item* label_i1 = dynamic_cast<Label_item*>(parsed_items.at(parsed_items.size() - 2))){
        cjump->label1 = label_i1->label_name;
      }
      if (Label_item* label_i2 = dynamic_cast<Label_item*>(parsed_items.back())){
        cjump->label2 = label_i2->label_name;
      }
      auto currentF = p.functions.back();
      currentF->instructions.push_back(cjump);
    }
  };

  template<> struct action < Cjump_fallthrough_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto cjump = new Cjump_fallthrough();
      cjump->c = parsed_comparisons.back();
      if (Label_item* label_i = dynamic_cast<Label_item*>(parsed_items.back())){
        cjump->label = label_i->label_name;
      }
      auto currentF = p.functions.back();
      currentF->instructions.push_back(cjump);
    }
  };

  template<> struct action < Goto_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto g = new Goto();
      if (Label_item* label_i = dynamic_cast<Label_item*>(parsed_items.back())){
        g->label = label_i->label_name;
      }
      auto currentF = p.functions.back();
      currentF->instructions.push_back(g);
    }
  };

  template<> struct action < Custom_func_call_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto c = new Custom_func_call();
      c->u = parsed_items.at(parsed_items.size()-2);
      if (Num_item* num_i = dynamic_cast<Num_item*>(parsed_items.back())){
        c->n = num_i->n;
      }
      auto currentF = p.functions.back();
      currentF->instructions.push_back(c);
    }
  };

  template<> struct action < Label_instruction_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto l = new Label_instruction();
      if (Label_item* label_i = dynamic_cast<Label_item*>(parsed_items.back())){
        l->label = label_i->label_name;
      }
      auto currentF = p.functions.back();
      currentF->instructions.push_back(l);
    }
  };

  template<> struct action < System_func_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto i = new Sys_func_item();
      i->func_name = in.string();
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
      if (Register_item* reg_i = dynamic_cast<Register_item*>(parsed_items.back())){
        i->reg = reg_i->register_name;
      } else if (Var_item* var_i = dynamic_cast<Var_item*>(parsed_items.back())){
        i->reg = var_i->var_name;
      }
      i->op = parsed_operations.back();
      auto currentF = p.functions.back();
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < At_arithmetic_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto at_a = new At_arithmetic();
      if (Register_item* reg_i = dynamic_cast<Register_item*>(parsed_items.at(parsed_items.size() - 4))){
        at_a->dest = reg_i->register_name;
      } else if (Var_item* var_i = dynamic_cast<Var_item*>(parsed_items.at(parsed_items.size() - 4))){
        at_a->dest = var_i->var_name;
      }

      if (Register_item* reg_i = dynamic_cast<Register_item*>(parsed_items.at(parsed_items.size() - 3))){
        at_a->r1 = reg_i->register_name;
      } else if (Var_item* var_i = dynamic_cast<Var_item*>(parsed_items.at(parsed_items.size() - 3))){
        at_a->r1 = var_i->var_name;
      }

      if (Register_item* reg_i = dynamic_cast<Register_item*>(parsed_items.at(parsed_items.size() - 2))){
        at_a->r2 = reg_i->register_name;
      } else if (Var_item* var_i = dynamic_cast<Var_item*>(parsed_items.at(parsed_items.size() - 2))){
        at_a->r2 = var_i->var_name;
      }

      if (Num_item* num_i = dynamic_cast<Num_item*>(parsed_items.back())){
        at_a->n = num_i->n;
      }
      auto currentF = p.functions.back();
      currentF->instructions.push_back(at_a);
    }
  };

  template<> struct action < System_func_call_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto s = new System_func_call();
      if (Sys_func_item* sys_i = dynamic_cast<Sys_func_item*>(parsed_items.back())){
        s->system_func = sys_i->func_name;
      }
      auto currentF = p.functions.back();
      currentF->instructions.push_back(s);
    }
  };

  template<> struct action < Assignment_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto a = new Assignment();
      a->d = parsed_destinations.back();
      a->op = parsed_operations.back();
      a->s = parsed_sources.back();
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
      parsed_operations.push_back(in.string());
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

  Program parse_function (char *fileName){

    /*
     * Check the grammar for some possible issues.
     * Check input starting from the function_rule.
     */
    pegtl::analyze< Functions_rule >();

    // Make empty program, to pass into existing action functions
    Program p;
    p.entryPointLabel = std::string("no_entry_point");
    file_input< > fileInput(fileName);

    // Parse, starting from the function_rule
    parse< Functions_rule, action >(fileInput, p);

    return p;
  }

  Program parse_spill (char *fileName){

    /*
     * Check the grammar for some possible issues.
     * Check input starting from the function_rule.
     */
    pegtl::analyze< Spill_rule >();

    // Make empty program, to pass into existing action functions
    Program p;
    p.entryPointLabel = std::string("no_entry_point");
    file_input< > fileInput(fileName);

    // Parse, starting from the spill_rule
    parse< Spill_rule, action >(fileInput, p);

    return p;
  }
}
