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
    std::map<std::string, std::string> m_sign = {
      {"<=", "setle"}, {"<", "setl"}, {">=", "setge"}, {">", "setg"}, {"=", "sete"}
    };

    bool is_int(std::string str){
      if (str[0] == '$'){
        return true;
      }
      return false;
    }

    void reverse(){
      left.swap(right);
      if (cmp_sign == "<="){
        cmp_sign = ">";
      } else if (cmp_sign == "<"){
        cmp_sign = ">=";
      }
    }

    int check(){
      if (is_int(left) && is_int(right)){
        int left_int = std::stoi(left.substr(1));
        int right_int = std::stoi(right.substr(1));
        if (cmp_sign  == "="){
          return left_int == right_int;
        } else if (cmp_sign == "<"){
          return left_int < right_int;
        } else {
          return left_int <= right_int;
        }
      }
      return -1;
    }

    std::string comp_to_string(bool set){
      if (is_int(right)){
        reverse();
      }
      std::string str = "\tcmpq " + right + ", " + left + "\n";
      if (set){
        str += "\t" + m_sign[cmp_sign];
      }
      return str;
    }
  };

  /*
   * ++,-- Instruction.
   * w++ | w--
   */
  struct Inc_or_dec : Instruction {
    std::string reg;
    std::string op;

    virtual std::string compile(){
      if (op == "++"){
        return "\tinc " + reg + "\n";
      } else  {
        return "\tdec " + reg + "\n";
      }
    }
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
    std::map<std::string, std::string> m_register = {
      {"%r10", "%r10b"}, {"%r11", "%r11b"}, {"%r12", "%r12b"}, {"%r13", "%r13b"},
      {"%r14", "%r14b"}, {"%r15", "%r15b"}, {"%r8", "%r8b"}, {"%r9", "%r9b"},
      {"%rax", "%al"}, {"%rbp", "%bpl"}, {"%rbx", "%bl"}, {"%rcx", "%cl"},
      {"%rdi", "%dil"}, {"%rdx", "%dl"}, {"%rsi", "%sil"}
    };

    virtual std::string compile(){
      int check_result = s.check();
      if (check_result == -1){
        std::string str = s.comp_to_string(true);
        std::string short_reg = m_register[d.value];
        return str + " " + short_reg + "\n\tmovzbq " + short_reg + ", " + d.value + "\n";
      } else {
        return "\tmovq $" + std::to_string(check_result) +  ", " + d.value + "\n";
      }
    }
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
