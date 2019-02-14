#pragma once

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <functional>
#include <utils.h>

namespace L3 {


  struct Item {
    Item_type type;
  };

  struct Variable : Item {
    type = VARIABLE;
    std::string name;
  };

  struct Number : Item {
    type = NUMBER;
    int64_t n;
  };

  struct label : Item {
    type = LABEL;
    std::string name;
  };

  struct Node {
    Item* value;
    std::vector<Node*> children;
    std::string operand;
  };

  struct Instruction {
    L3::Instruction_type type;
  }

  struct Instruction_s : Instruction {
    type = L3::Instruction_type::S;
    Variable* dest;
    Item* source;
  }

  struct Instruction_op : Instruction {
    type = L3::Instruction_type::OP;
    Variable* dest;
    std::string op;
    Item* t1;
    Item* t2;
  }

  struct Instruction_cmp : Instruction {
    type = L3::Instruction_type::CMP;
    Variable* dest;
    std::string cmp;
    Item* t1;
    Item* t2;
  }

  struct Instruction_load : Instruction {
    type = L3::Instruction_type::LOAD;
    Variable* dest;
    Variable* source;
  }

  struct Instruction_store : Instruction {
    type = L3::Instruction_type::STORE;
    Variable* dest;
    Item* source;
  }

  struct Instruction_goto : Instruction {
    // br label
    Label* label;
  }

  struct Function{
    std::string name;
    int64_t arguments;
    int64_t locals;
    std::vector<Instruction *> instructions;
  };

  struct Program{
    std::vector<Function *> functions;
  };
}
