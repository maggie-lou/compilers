#pragma once

#include <vector>

namespace L1 {
  /*
   * Register.
   */
  struct Register{
    std::string name;
    int64_t value;
  };

  /*
   * Instruction interface.
   */
  struct Instruction{
  };

  /*
   * Instructions.
   */
  struct Instruction_ret : Instruction{
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
    Register r;
    int64_t offset;
  };

  struct Item {
    std::string labelName;
    Register r;
    int64_t n;
    Address address;
    std::string op;
    std::string cmp_sign;

    // 1: label
    // 2: register
    // 3: number
    // 4: address
    int64_t type;
  };

  struct Comparison {
    Item left;
    Item right;
    std::string cmp_sign;
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
    Item u;
    int64_t n;
  };

  /*
   * System function call Instruction.
   * call u N
   */
  struct System_func_call : Instruction {
    std::string system_func;
    int64_t n;
  };

  /*
   * Program.
   */
  struct Program{
    std::string entryPointLabel;
    std::vector<Function *> functions;
    std::vector<Register *> registers;
  };
}
