
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

#include <L1.h>
#include <parser.h>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace L1 {

  /*
   * Data required to parse
   */
  std::vector<Item> parsed_destinations;
  std::vector<std::string> parsed_operations;
  std::vector<Item> parsed_sources;
  std::vector<Item> parsed_items;
  std::vector<Register> parsed_address_xs;
  std::vector<int64_t> parsed_address_ms;
  std::vector<std::string> parsed_cmp_signs;
  std::vector<Item> parsed_cmp_lefts;
  std::vector<Item> parsed_cmp_rights;

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

  struct E_rule:
    pegtl::sor<
      pegtl::one<'0'>,
      pegtl::one<'2'>,
      pegtl::one<'4'>,
      pegtl::one<'8'>
    > { };

  struct Cmp_rule:
    pegtl::sor<
      pegtl::seq<
        pegtl::one<'<'>,
        pegtl::one<'='>
      >,
      pegtl::seq<
        pegtl::one<'<'>
      >,
      pegtl::seq<
        pegtl::one<'='>
      >
    > { };

  struct Sop_rule:
    pegtl::sor<
      pegtl::seq<
        pegtl::one<'<'>,
        pegtl::one<'<'>,
        pegtl::one<'='>
      >,
      pegtl::seq<
        pegtl::one<'>'>,
        pegtl::one<'>'>,
        pegtl::one<'='>
      >
    > { };

  struct Aop_rule:
    pegtl::sor<
      pegtl::seq<
        pegtl::one<'+'>,
        pegtl::one<'='>
      >,
      pegtl::seq<
        pegtl::one<'-'>,
        pegtl::one<'='>
      >,
      pegtl::seq<
        pegtl::one<'*'>,
        pegtl::one<'='>
      >,
      pegtl::seq<
        pegtl::one<'&'>,
        pegtl::one<'='>
      >
    > { };

  struct Assign_rule:
    pegtl::seq<
      pegtl::one<'<'>,
      pegtl::one<'-'>
    > { };

  struct Instruction_return_rule:
    pegtl::seq<
      str_return
    > { };

  struct Sx_rule:
    pegtl::seq<
      pegtl::one<'r'>,
      pegtl::one<'c'>,
      pegtl::one<'x'>
    > {};

  struct A_other_rule:
    pegtl::sor<
      pegtl::seq<
        pegtl::one<'r'>,
        pegtl::one<'d'>,
        pegtl::one<'i'>
      >,
      pegtl::seq<
        pegtl::one<'r'>,
        pegtl::one<'d'>,
        pegtl::one<'x'>
      >,
      pegtl::seq<
        pegtl::one<'r'>,
        pegtl::one<'8'>
      >,
      pegtl::seq<
        pegtl::one<'r'>,
        pegtl::one<'9'>
      >,
      pegtl::seq<
        pegtl::one<'r'>,
        pegtl::one<'s'>,
        pegtl::one<'i'>
      >
    > {};

  struct A_rule:
    pegtl::sor<
      Sx_rule,
      A_other_rule
    > {};

  struct W_other_rule:
    pegtl::sor<
      pegtl::seq<
        pegtl::one<'r'>,
        pegtl::one<'b'>,
        pegtl::one<'x'>
      >,
      pegtl::seq<
        pegtl::one<'r'>,
        pegtl::one<'a'>,
        pegtl::one<'x'>
      >,
      pegtl::seq<
        pegtl::one<'r'>,
        pegtl::one<'b'>,
        pegtl::one<'p'>
      >,
      pegtl::seq<
        pegtl::one<'r'>,
        pegtl::one<'1'>,
        pegtl::one<'0'>
      >,
      pegtl::seq<
        pegtl::one<'r'>,
        pegtl::one<'1'>,
        pegtl::one<'1'>
      >,
      pegtl::seq<
        pegtl::one<'r'>,
        pegtl::one<'1'>,
        pegtl::one<'2'>
      >,
      pegtl::seq<
        pegtl::one<'r'>,
        pegtl::one<'1'>,
        pegtl::one<'3'>
      >,
      pegtl::seq<
        pegtl::one<'r'>,
        pegtl::one<'1'>,
        pegtl::one<'4'>
      >,
      pegtl::seq<
        pegtl::one<'r'>,
        pegtl::one<'1'>,
        pegtl::one<'5'>
      >
    > {};

  struct W_rule:
    pegtl::sor<
      W_other_rule,
      A_rule
    > {};

  struct X_other_rule:
    pegtl::seq<
      pegtl::one<'r'>,
      pegtl::one<'s'>,
      pegtl::one<'p'>
    > {};

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

  struct Address_x_rule:
    pegtl::seq<
      X_rule
    > { };

  struct Address_m_rule:
    pegtl::seq<
      M_rule
    > { };

  struct Address_rule:
    pegtl::seq<
      pegtl::one<'m'>,
      pegtl::one<'e'>,
      pegtl::one<'m'>,
      seps,
      Address_x_rule,
      seps,
      Address_m_rule
    > { };

  struct Source_rule:
    pegtl::sor<
      S_rule,
      Address_rule,
    > { };

  struct Destination_rule:
    pegtl::sor<
      W_rule,
      Address_rule
    > { };

  struct Operation_rule:
    pegtl::sor<
      Assign_rule,
      Aop_rule,
      Sop_rule
    > { };

  struct Assignment_rule:
    pegtl::seq<
      Destination_rule,
      seps,
      Operation_rule,
      seps,
      Source_rule
    > { };

  // t cmp t
  struct Comparison_rule:
    pegtl::seq<
      Comparison_left_rule,
      seps,
      Cmp_rule,
      seps,
      Comparison_right_rule
    > { };

  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<Instruction_return_rule>            , Instruction_return_rule             >,
      pegtl::seq< pegtl::at<Assignment_rule>            , Assignment_rule             >
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
        p.entryPointLabel = in.string();
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
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Label_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      Item i;
      i.labelName = in.string();
      i.type = 1;
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Number_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      Item i;
      i.type = 3;
      i.labelName = std::stoll(in.string());
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Sx_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      Register newR;
      newR.name = in.string();

      Item i;
      i.type = 2;
      i.r = newR;

      parsed_items.push_back(i);
    }
  };

  template<> struct action < A_other_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      Register newR;
      newR.name = in.string();

      Item i;
      i.type = 2;
      i.r = newR;

      parsed_items.push_back(i);
    }
  };

  template<> struct action < W_other_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      Register newR;
      newR.name = in.string();

      Item i;
      i.type = 2;
      i.r = newR;

      parsed_items.push_back(i);
    }
  };

  template<> struct action < X_other_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      Register newR;
      newR.name = in.string();

      Item i;
      i.type = 2;
      i.r = newR;

      parsed_items.push_back(i);
    }
  };

  template<> struct action < Address_x_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto i = parsed_items.back();
      auto x = i.r;
      parsed_address_xs.push_back(x);
    }
  };

  template<> struct action < Address_m_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
    auto i = parsed_items.back();
    auto m = i.n;
    parsed_address_ms.push_back(m);
    }
  };

  template<> struct action < Address_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      auto x = parsed_address_xs.back();
      auto m = parsed_address_ms.back();
      Address address;
      address.offset = m;
      address.r = x;

      Item i;
      i.type = 4;
      i.address = address;
      parsed_items.push_back(i);
    }
  };

  template<> struct action < Comparison_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
      Comparison comparison;
      comparison.left = parsed_cmp_lefts.back();
      comparison.right = parsed_cmp_rights.back();
      comparison.cmp_sign = parsed_cmp_signs.back();

      Item i;
      i.type = 5;
      i.comparison = comparison;
      parsed_items.push_back(i);
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

  template<> struct action < Assign_rule > {
    template< typename Input >
	static void apply( const Input & in, Program & p){
    parsed_operations.push_back(in.string());
    }
  };

  template<> struct action < Cmp_rule > {
    template< typename Input >
  static void apply( const Input & in, Program & p){
      parsed_cmp_signs.push_back(in.string());
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
