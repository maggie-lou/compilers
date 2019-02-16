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

    virtual void match(Node* root, std::vector<Node*> &leaves, std::stack<std::string> &ins){
    }
  };

  struct Tile_assign : Tile {
    Tile_s(){
      cost = 1;
      size = 2;
    }

    virtual void match(Node* root, std::vector<Node*> &leaves, std::stack<std::string> &ins){
      Item* root_val = root->value;
      if ((root_val->type == Item_type::VARIABLE)&&(root->operand_type == Instruction_type::ASSIGN)&&(root->children.size()==1)){
        Item* child_val = root->children[0]->value;
        if (root->children[0]->value->type == Item_type::VARIABLE||root->children[0]->value->type == Item_type::LABEL||root->children[0]->value->type == Item_type::NUMBER){
          ins.push("\t" + root_val->to_str() + " <- " + root->children[0]->value->to_str() + "\n");
          if (!root->children[0]->children.empty()) remains.push_back(root->children[0]);
          return true;
        }
      }
      remains.push_back(root);
      return false;
    }
  };

  struct Tile_op : Tile {
    Tile_s(){
      cost = 2;
      size = 3;
    }

    virtual void match(Node* root, std::vector<Node*> &leaves, std::stack<std::string> &ins){
      Item* root_val = root->value;
      if ((root_val->type == Item_type::VARIABLE)&&(root->operand_type == Instruction_type::OP)&&(root_val->children.size()==2)){
        Item* t1_val = root->children[0]->value;
        Item* t2_val = root->children[1]->value;
        if ((t1_val->type == Item_type::VARIABLE||t1_val->type == Item_type::NUMBER)
            &&(t2_val->type == Item_type::VARIABLE||t2_val->type == Item_type::NUMBER)){

          ins.push("\t"+t1_val->to_str()+" <- "+t1_val->to_str()+" "+root->op+" "+t2_val->to_str()+"\n");
          ins.push("\t"+root_val->to_str()+" <- "+t1_val->to_str()+"\n");
          if (!root->children[0]->children.empty()) remains.push_back(root->children[0]);
          if (!root->children[1]->children.empty()) remains.push_back(root->children[1]);
          return true;
        }
      }
      remains.push_back(root);
      return false;
    }
  };

  struct Tile_cmp : Tile {
    Tile_s(){
      cost = 1;
      size = 3;
    }

    virtual void match(Node* root, std::vector<Node*> &leaves, std::stack<std::string> &ins){
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
            ins.push(root->val->to_str()+" <- "+t2_val->to_str()+" "+root->cmp+" "+t1_val->to_str()+"\n");
          } else {
            ins.push(root->val->to_str()+" <- "+t1_val->to_str()+" "+root->cmp+" "+t2_val->to_str()+"\n");
          }
          if (!root->children[0]->children.empty()) remains.push_back(root->children[0]);
          if (!root->children[1]->children.empty()) remains.push_back(root->children[1]);
          return true;
        }
      }
      remains.push_back(root);
      return false;
    }
  };

  struct Tile_load : Tile {
    Tile_s(){
      cost = 1;
      size = 2;
    }

    virtual void match(Node* root, std::vector<Node*> &leaves, std::stack<std::string> &ins){
      Item* root_val = root->value;
      if ((root_val->type == Item_type::VARIABLE)&&(root->operand_type == Instruction_type::LOAD)&&(root_val->children.size()==1)){
        Item* child_val = root->children[0]->value;
        if (child_val->type == Item_type::VARIABLE){
          ins.push("\t"+root_val->to_str()+" <- mem "+child_val->to_str()+"  0\n");
          if (!root->children[0]->children.empty()) remains.push_back(root->children[0]->children);
          return true;
        }
      }
      remains.push_back(root);
      return false;
    }
  };

  struct Tile_store : Tile {
    Tile_s(){
      cost = 1;
      size = 2;
    }

    virtual void match(Node* root, std::vector<Node*> &leaves, std::stack<std::string> &ins){
      Item* root_val = root->value;
      if ((root_val->type == Item_type::VARIABLE)&&(root->operand_type == Instruction_type::STORE)&&(root_val->children.size()==1)){
        Item* child_val = root->children[0]->value;
        if (child_val->type == Item_type::VARIABLE){
          ins.push("\tmem "+root_val->to_str()+" 0 <- "+child_val->to_str()+"\n");
          if (!root->children[0]->children.empty()) remains.push_back(root->children[0]->children);
          return true;
        }
      }
      remains.push_back(root);
      return false;
    }
  };

  struct Tile_goto : Tile {
    Tile_s(){
      cost = 1;
      size = 1;
    }

    virtual void match(Node* root, std::vector<Node*> &leaves, std::stack<std::string> &ins){
      Item* root_val = root->value;
      if ((root_val->type == Item_type::LABEL)&&(root->operand_type == Instruction_type::GOTO)){
        ins.push("\tgoto "+root_val->to_str()+"\n");
        return true;
      }
      remains.push_back(root);
      return false;
    }
  };

  struct Tile_label : Tile {
    Tile_s(){
      cost = 1;
      size = 1;
    }

    virtual void match(Node* root, std::vector<Node*> &leaves, std::stack<std::string> &ins){
      Item* root_val = root->value;
      if ((root_val->type == Item_type::LABEL)&&(root->operand_type == Instruction_type::LABEL)){
        ins.push("\t"+root_val->to_str()+"\n");
        return true;
      }
      remains.push_back(root);
      return false;
    }
  };

  struct Tile_jump : Tile {
    Tile_s(){
      cost = 1;
      size = 1;
    }

    virtual void match(Node* root, std::vector<Node*> &leaves, std::stack<std::string> &ins){
      Item* root_val = root->value;
      if ((root_val->type == Item_type::VARIABLE)&&(root->operand_type == Instruction_type::JUMP)&&(root->children.size()==2)){
        Item* l1_val = root->children[0]->value;
        Item* l2_val = root->children[1]->value;
        // 0 is false, 1 is true
        if (l1_val->type == Item_type::LABEL && l2_val->type == Item_type::LABEL){
          ins.push("\tcjump "+root_val->to_str()+" = 1 "+l1_val->to_str()+" "+l2_val->to_str()+"\n");
          return true;
        }
      }
      remains.push_back(root);
      return false;
    }
  };

  struct Tile_ret_void : Tile {
    Tile_s(){
      cost = 1;
      size = 0;
    }

    virtual void match(Node* root, std::vector<Node*> &leaves, std::stack<std::string> &ins){
      if (root->operand_type == Instruction_type::RETVOID){
        ins.push("\treturn\n");
        return true;
      }
      remains.push_back(root);
      return false;
    }
  };

  struct Tile_ret : Tile {
    Tile_s(){
      cost = 2;
      size = 1;
    }

    virtual void match(Node* root, std::vector<Node*> &leaves, std::stack<std::string> &ins){
      Item* root_val = root->value;
      if ((root->operand_type == Instruction_type::RETVOID)&&(root_val->type==Item_type::VARIABLE)){
        ins.push("\treturn\n");
        ins.push("\trax <- "+root_val->to_str()+"\n");
        // TODO unfinished

      }

    }
  };


}
