#pragma once

#include <vector>
#include <map>
#include <string>

namespace L1 {

  /*
   * Instruction interface.
   */
  struct Instruction{
    virtual std::string compile(){
      return "";
    };
  };

  /*
   * Instructions.
   */
  struct Instruction_ret : Instruction{
    virtual std::string compile(){
      return "\tretq\n";
    }
  };

  /*
   * Function.
   */
  struct Function{
    std::string name;
    int64_t arguments;
    int64_t locals;
    std::vector<Instruction *> instructions;
  };


  struct Address{
    std::string r;
    std::string offset;
    std::string address_to_string(){
      return offset.substr(1)+"("+r+")";
    }
  };

  struct Item {
    std::string value;
    Address address;
    bool is_address;

    std::string item_to_string(){
      if (is_address){
        return address.address_to_string();
      } else {
        return value;
      }
    }
  };

  struct Comparison {
    std::string left;
    std::string right;
    std::string cmp_sign;
  };

  /*
   * ++,-- Instruction.
   * w++ | w--
   */
  struct Inc_or_dec : Instruction {
    std::string reg;
    std::string op;
  };

  /*
   * @ Instruction.
   * w @ w w E
   */
  struct At_arithmetic : Instruction {
    std::string dest;
    std::string r1;
    std::string r2;
    std::string n;
  };

  /*
   * Assignment Instruction.
   * w <- s | w <- mem x M | mem x M <- s
   * w aop t | w sop sx | w sop N |
   * mem x M += t | mem x M -= t | w += mem x M | w -= mem x M |
   */
  struct Assignment : Instruction {
    Item d;
    Item s;
    std::string op;

    std::map<std::string, int> m = {
      {"<-", 1}, {"+=", 2}, {"-=", 3}, {"*=", 4},
      {"&=", 5}, {">>=", 6}, {"<<=", 7}
    };

    virtual std::string compile(){
      switch (m[op]) {
        case 1:
          return "\tmovq "+s.item_to_string()+", "+d.item_to_string()+"\n";
        case 2:
          return "\taddq "+s.item_to_string()+", "+d.item_to_string()+"\n";
        case 3:
          return "\tsubq "+s.item_to_string()+", "+d.item_to_string()+"\n";
        case 4:
          return "\timulq "+s.item_to_string()+", "+d.item_to_string()+"\n";
        case 5:
          return "\tandq "+s.item_to_string()+", "+d.item_to_string()+"\n";
        case 6:
          return "\tsarq "+s.item_to_string()+", "+d.item_to_string()+"\n";
        case 7:
          return "\tsalq "+s.item_to_string()+", "+d.item_to_string()+"\n";
        default:
          return "";
      }
    }
  };

  /*
   * Assignment Instruction.
   * w <- t cmp t
   */
  struct AssignmentCmp : Instruction {
    Item d;
    Comparison s;
  };

  /*
   * Cjump Instruction.
   * cjump t cmp t label label
   */
  struct Cjump : Instruction {
    Comparison c;
    std::string label1;
    std::string label2;
  };

  /*
   * Cjump Fallthrough Instruction.
   * cjump t cmp t label
   */
  struct Cjump_fallthrough : Instruction {
    Comparison c;
    std::string label;
  };

  /*
   * Label Instruction.
   * label
   */
  struct Label_instruction : Instruction {
    std::string label;
  };

  /*
   * Goto Instruction.
   * goto label
   */
  struct Goto : Instruction {
    std::string label;
  };

  /*
   * Custom function call Instruction.
   * call u N
   */
  struct Custom_func_call : Instruction {
    std::string u;
    std::string n;
  };

  /*
   * System function call Instruction.
   * call u N
   */
  struct System_func_call : Instruction {
    std::string system_func;
    std::string n;
  };

  /*
   * Program.
   */
  struct Program{
    std::string entryPointLabel;
    std::vector<Function *> functions;
  };
}
