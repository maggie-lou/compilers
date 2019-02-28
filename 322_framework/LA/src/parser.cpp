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

#include <LA.h>
#include <parser.h>
#include <utils.h>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace LA {

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

  struct Label_rule:
    label {};

  struct Number_rule:
    number {};

  struct Var_rule:
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
      Var_rule,
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
      Var_rule,
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
      Var_rule,
      seps,
      pegtl::one<'('>,
      seps,
      Args_rule,
      seps,
      pegtl::one<')'>
    > {};

  struct Instruction_print:
    pegtl::seq<
      pegtl::string<'p','r','i','n','t'>,
      seps,
      pegtl::one<'('>,
      seps,
      T_rule,
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
      pegtl::seq< pegtl::at<Instruction_tuple_rule>, Instruction_tuple_rule >,
      pegtl::seq< pegtl::at<Instruction_print_rule>, Instruction_print_rule >,
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
