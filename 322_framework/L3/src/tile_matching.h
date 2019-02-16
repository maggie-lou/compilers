#pragma once

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <functional>
#include <utils.h>
#include <L3.h>
#include <stack>

namespace L3 {
  struct Tile {
    int64_t cost;
    int64_t size;

    virtual bool match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions){
    }
  };

  struct Tile_assign : Tile {
    Tile_assign(){
      cost = 1;
      size = 2;
    }

    virtual void match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions){
      Item* root_val = root->value;
      if ((root_val->type == Item_type::VARIABLE)&&(root->operand_type == Instruction_type::ASSIGN)&&(root->children.size()==1)){
        Item* child_val = root->children[0]->value;
        if (root->children[0]->value->type == Item_type::VARIABLE||root->children[0]->value->type == Item_type::LABEL||root->children[0]->value->type == Item_type::NUMBER){
          L2_instructions.push("\t" + root_val->to_string() + " <- " + root->children[0]->value->to_string() + "\n");
          if (!root->children[0]->children.empty()) unmatched.push_back(root->children[0]);
          return true;
        }
      }
      unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_op : Tile {
    Tile_op(){
      cost = 2;
      size = 3;
    }

    virtual void match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions){
      Item* root_val = root->value;
      if ((root_val->type == Item_type::VARIABLE)&&(root->operand_type == Instruction_type::OP)&&(root_val->children.size()==2)){
        Item* t1_val = root->children[0]->value;
        Item* t2_val = root->children[1]->value;
        if ((t1_val->type == Item_type::VARIABLE||t1_val->type == Item_type::NUMBER)
            &&(t2_val->type == Item_type::VARIABLE||t2_val->type == Item_type::NUMBER)){

          L2_instructions.push("\t"+t1_val->to_string()+" <- "+t1_val->to_string()+" "+root->op+" "+t2_val->to_string()+"\n");
          L2_instructions.push("\t"+root_val->to_string()+" <- "+t1_val->to_string()+"\n");
          if (!root->children[0]->children.empty()) unmatched.push_back(root->children[0]);
          if (!root->children[1]->children.empty()) unmatched.push_back(root->children[1]);
          return true;
        }
      }
      unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_cmp : Tile {
    Tile_cmp(){
      cost = 1;
      size = 3;
    }

    virtual void match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions){
      Item* root_val = root->value;
      if ((root_val->type == Item_type::VARIABLE)&&(root->operand_type == Instruction_type::CMP)&&(root_val->children.size()==2)){
        Item* t1_val = root->children[0]->value;
        Item* t2_val = root->children[1]->value;
        if ((t1_val->type == Item_type::VARIABLE||t1_val->type == Item_type::NUMBER)
            &&(t2_val->type == Item_type::VARIABLE||t2_val->type == Item_type::NUMBER)){
          bool flip = false;
          if (root->cmp == ">"){
            root->cmp = "<";
            flip = true;
          } else if (root->cmp == ">="){
            root->cmp = "<=";
            flip = true;
          }
          if (flip){
            L2_instructions.push(root->val->to_string()+" <- "+t2_val->to_string()+" "+root->cmp+" "+t1_val->to_string()+"\n");
          } else {
            L2_instructions.push(root->val->to_string()+" <- "+t1_val->to_string()+" "+root->cmp+" "+t2_val->to_string()+"\n");
          }
          if (!root->children[0]->children.empty()) unmatched.push_back(root->children[0]);
          if (!root->children[1]->children.empty()) unmatched.push_back(root->children[1]);
          return true;
        }
      }
      unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_load : Tile {
    Tile_load(){
      cost = 1;
      size = 2;
    }

    virtual void match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions){
      Item* root_val = root->value;
      if ((root_val->type == Item_type::VARIABLE)&&(root->operand_type == Instruction_type::LOAD)&&(root_val->children.size()==1)){
        Item* child_val = root->children[0]->value;
        if (child_val->type == Item_type::VARIABLE){
          L2_instructions.push("\t"+root_val->to_string()+" <- mem "+child_val->to_string()+"  0\n");
          if (!root->children[0]->children.empty()) unmatched.push_back(root->children[0]->children);
          return true;
        }
      }
      unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_store : Tile {
    Tile_store(){
      cost = 1;
      size = 2;
    }

    virtual void match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions){
      Item* root_val = root->value;
      if ((root_val->type == Item_type::VARIABLE)&&(root->operand_type == Instruction_type::STORE)&&(root_val->children.size()==1)){
        Item* child_val = root->children[0]->value;
        if (child_val->type == Item_type::VARIABLE){
          L2_instructions.push("\tmem "+root_val->to_string()+" 0 <- "+child_val->to_string()+"\n");
          if (!root->children[0]->children.empty()) unmatched.push_back(root->children[0]->children);
          return true;
        }
      }
      unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_goto : Tile {
    Tile_goto(){
      cost = 1;
      size = 1;
    }

    virtual void match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions){
      Item* root_val = root->value;
      if ((root_val->type == Item_type::LABEL)&&(root->operand_type == Instruction_type::GOTO)){
        L2_instructions.push("\tgoto "+root_val->to_string()+"\n");
        return true;
      }
      unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_label : Tile {
    Tile_label(){
      cost = 1;
      size = 1;
    }

    virtual void match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions){
      Item* root_val = root->value;
      if ((root_val->type == Item_type::LABEL)&&(root->operand_type == Instruction_type::LABEL)){
        L2_instructions.push("\t"+root_val->to_string()+"\n");
        return true;
      }
      unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_jump : Tile {
    Tile_jump(){
      cost = 1;
      size = 1;
    }

    virtual void match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions){
      Item* root_val = root->value;
      if ((root_val->type == Item_type::VARIABLE)&&(root->operand_type == Instruction_type::JUMP)&&(root->children.size()==2)){
        Item* l1_val = root->children[0]->value;
        Item* l2_val = root->children[1]->value;
        // 0 is false, 1 is true
        if (l1_val->type == Item_type::LABEL && l2_val->type == Item_type::LABEL){
          L2_instructions.push("\tcjump "+root_val->to_string()+" = 1 "+l1_val->to_string()+" "+l2_val->to_string()+"\n");
          return true;
        }
      }
      unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_ret_void : Tile {
    Tile_ret_void(){
      cost = 1;
      size = 0;
    }

    virtual void match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions){
      if (root->operand_type == Instruction_type::RETVOID){
        L2_instructions.push("\treturn\n");
        return true;
      }
      unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_ret : Tile {
    Tile_ret(){
      cost = 2;
      size = 1;
    }

    virtual void match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions){
      Item* root_val = root->value;
      if (root->operand_type == Instruction_type::RET){
        L2_instructions.push("\treturn\n");
        L2_instructions.push("\trax <- "+root_val->to_string()+"\n");
        if (!root->children.empty()) {
          unmatched.insert(unmatched.end(), root->children.begin(), root->children.end());
        }
        return true;
      }
      unmatched.push_back(root);
      return false;

    }
  };

  struct Tile_call : Tile {
    Tile_call(){
      cost = -1; // Dependent on number of arguments
      size = -1; // Dependent on number of arguments
    }

    virtual void match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions){
      bool is_call = root->type == Instruction_type::CALL;
      if (is_call) {
        L2_instructions.push("\tcall " + root->value->to_string() + " " + root->children.size() + "\n");
        for (int i=0; i<root->children.size(); i++) {
          Node* child = root->children[i];
          if (i >= argument_registers.size()) {
            int stack_loc = -16 - 8 * (i - argument_register.size());
            L2_instructions.push("\tmem rsp " + to_string(stack_loc) + " <- " + child->value->to_string());
          } else {
            string arg_register = argument_registers[i];
            L2_instructions.push("\t" + argument_registers[i] + " <- " + child->value->to_string());
          }
          unmatched.insert(unmatched.end(), child->children.begin(), child->children.end());
        }
        return true;
      }

      unmatched.insert(root);
      return false;
    }
  };

  struct Tile_call_store : Tile {
    Tile_call_store(){
      cost = -1; // Dependent on number of arguments
      size = -1; // Dependent on number of arguments
    }

    virtual void match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions){
      bool is_call_store = root->type == Instruction_type::CALLSTORE;
      if (is_call_store) {
        L2_instructions.push("\t" + t->value->to_string() + " <- rax");
        L2_instructions.push("\tcall " + root->value->to_string() + " " + root->children.size() + "\n");
        for (int i=0; i<root->children.size(); i++) {
          Node* child = root->children[i];
          if (i >= argument_registers.size()) {
            int stack_loc = -16 - 8 * (i - argument_register.size());
            L2_instructions.push("\tmem rsp " + to_string(stack_loc) + " <- " + child->value->to_string());
          } else {
            string arg_register = argument_registers[i];
            L2_instructions.push("\t" + argument_registers[i] + " <- " + child->value->to_string());
          }
          unmatched.insert(unmatched.end(), child->children.begin(), child->children.end());
        }
        return true;
      }

      unmatched.insert(root);
      return false;
    }
  };
}
