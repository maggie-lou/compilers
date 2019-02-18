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
  std::stack<std::string> generate_l2_instructions(Node* trees, std::string longest_label_name, int64_t &label_count);

  struct Tile {
    int64_t cost;
    int64_t size;

    virtual bool match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions, std::string longest_label_name, int64_t &label_count){
      cout <<"In default"<<endl;
      return false;
    }
  };

  struct Tile_assign : Tile {
    Tile_assign(){
      cost = 1;
      size = 2;
    }

    virtual bool match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions, std::string longest_label_name, int64_t &label_count){
      cout << "in tile assign\n";
      if (root->operand_type == Instruction_type::ASSIGN){
        Item* root_val = root->value;
        if ((root_val->type == Item_type::VARIABLE)&&(root->children.size()==1)){
          if (root->children[0]->value){
            Item* child_val = root->children[0]->value;
            if (child_val->type == Item_type::VARIABLE||child_val->type == Item_type::LABEL||child_val->type == Item_type::NUMBER){
              L2_instructions.push("\t" + root_val->to_string() + " <- " + child_val->to_string() + "\n");
              if (!root->children[0]->children.empty()) unmatched.push_back(root->children[0]);
              return true;
            }
          }
        }
      }
      //unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_op : Tile {
    Tile_op(){
      cost = 2;
      size = 3;
    }

    virtual bool match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions, std::string longest_label_name, int64_t &label_count){
      cout << "in tile op\n";
      if (root->operand_type == Instruction_type::OP){
        Item* root_val = root->value;
        if ((root_val->type == Item_type::VARIABLE)&&(root->children.size()==2)){
          if (root->children[0]->value && root->children[1]->value){
            Item* t1_val = root->children[0]->value;
            Item* t2_val = root->children[1]->value;
            if ((t1_val->type == Item_type::VARIABLE||t1_val->type == Item_type::NUMBER)
                &&(t2_val->type == Item_type::VARIABLE||t2_val->type == Item_type::NUMBER)){

              L2_instructions.push("\t"+root_val->to_string()+" <- "+t1_val->to_string()+"\n");
              L2_instructions.push("\t"+t1_val->to_string()+" "+root->op_name+"= "+t2_val->to_string()+"\n");

              if (!root->children[0]->children.empty()) unmatched.push_back(root->children[0]);
              if (!root->children[1]->children.empty()) unmatched.push_back(root->children[1]);
              return true;
            }
          }
        }
      }
      // unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_cmp : Tile {
    Tile_cmp(){
      cost = 1;
      size = 3;
    }

    virtual bool match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions, std::string longest_label_name, int64_t &label_count){
      cout << "in tile cmp\n";
      if (root->operand_type == Instruction_type::CMP){
        Item* root_val = root->value;
        if ((root_val->type == Item_type::VARIABLE)&&(root->children.size()==2)){
          if (root->children[0]->value && root->children[1]->value){
            Item* t1_val = root->children[0]->value;
            Item* t2_val = root->children[1]->value;
            if ((t1_val->type == Item_type::VARIABLE||t1_val->type == Item_type::NUMBER)
                &&(t2_val->type == Item_type::VARIABLE||t2_val->type == Item_type::NUMBER)){
              bool flip = false;
              if (root->op_name == ">"){
                root->op_name = "<";
                flip = true;
              } else if (root->op_name == ">="){
                root->op_name = "<=";
                flip = true;
              }
              if (flip){
                L2_instructions.push("\t"+root->value->to_string()+" <- "+t2_val->to_string()+" "+root->op_name+" "+t1_val->to_string()+"\n");
              } else {
                L2_instructions.push("\t"+root->value->to_string()+" <- "+t1_val->to_string()+" "+root->op_name+" "+t2_val->to_string()+"\n");
              }
              if (!root->children[0]->children.empty()) unmatched.push_back(root->children[0]);
              if (!root->children[1]->children.empty()) unmatched.push_back(root->children[1]);
              return true;
            }
          }
        }
      }

      //unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_load : Tile {
    Tile_load(){
      cost = 1;
      size = 2;
    }

    virtual bool match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions, std::string longest_label_name, int64_t &label_count){
      cout << "in tile load\n";
      if (root->operand_type == Instruction_type::LOAD){
        Item* root_val = root->value;
        if ((root_val->type == Item_type::VARIABLE)&&(root->children.size()==1)){
          if (root->children[0]->value){
            Item* child_val = root->children[0]->value;
            if (child_val->type == Item_type::VARIABLE){
              L2_instructions.push("\t"+root_val->to_string()+" <- mem "+child_val->to_string()+"  0\n");
              if (!root->children[0]->children.empty()) unmatched.push_back(root->children[0]);
              return true;
            }
          }
        }
      }

      //unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_store : Tile {
    Tile_store(){
      cost = 1;
      size = 2;
    }

    virtual bool match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions, std::string longest_label_name, int64_t &label_count){
      cout << "in tile store\n";
      if (root->operand_type == Instruction_type::STORE){
        Item* root_val = root->value;
        if ((root_val->type == Item_type::VARIABLE)&&(root->children.size()==1)){
          if (root->children[0]->value){
            Item* child_val = root->children[0]->value;
            if (child_val->type == Item_type::VARIABLE){
              L2_instructions.push("\tmem "+root_val->to_string()+" 0 <- "+child_val->to_string()+"\n");
              if (!root->children[0]->children.empty()) unmatched.push_back(root->children[0]);
              return true;
            }
          }
        }
      }

      //unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_goto : Tile {
    Tile_goto(){
      cost = 1;
      size = 1;
    }

    virtual bool match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions, std::string longest_label_name, int64_t &label_count){
      cout << "in tile goto\n";
      if (root->operand_type == Instruction_type::GOTO){
        Item* root_val = root->value;
        if ((root_val->type == Item_type::LABEL)){
          L2_instructions.push("\tgoto "+root_val->to_string()+"\n");
          return true;
        }
      }
      //unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_label : Tile {
    Tile_label(){
      cost = 1;
      size = 1;
    }

    virtual bool match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions, std::string longest_label_name, int64_t &label_count){
      cout << "in tile label\n";
      if (root->operand_type == Instruction_type::LABELI){
        Item* root_val = root->value;
        if ((root_val->type == Item_type::LABEL)){
          L2_instructions.push("\t"+root_val->to_string()+"\n");
          return true;
        }
      }
      //unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_jump : Tile {
    Tile_jump(){
      cost = 1;
      size = 1;
    }

    virtual bool match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions, std::string longest_label_name, int64_t &label_count){
      cout << "in tile jump\n";
      if (root->operand_type == Instruction_type::JUMP){
        Item* root_val = root->value;
        if ((root_val->type == Item_type::VARIABLE)&&(root->children.size()==1)){
          if (root->children[0]->value){
            Item* l_val = root->children[0]->value;
            // 0 is false, 1 is true
            if (l_val->type == Item_type::LABEL){
              L2_instructions.push("\tcjump "+root_val->to_string()+" = 1 "+l_val->to_string()+"\n");
              return true;
            }
          }
        }
      }

      //unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_ret_void : Tile {
    Tile_ret_void(){
      cost = 1;
      size = 0;
    }

    virtual bool match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions, std::string longest_label_name, int64_t &label_count){
      cout << "in tile ret void\n";
      if (root->operand_type == Instruction_type::RETVOID){
        L2_instructions.push("\treturn\n");
        return true;
      }
      //unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_ret : Tile {
    Tile_ret(){
      cost = 2;
      size = 1;
    }

    virtual bool match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions, std::string longest_label_name, int64_t &label_count){
      cout << "in tile ret\n";
      if (root->operand_type == Instruction_type::RET){
        L2_instructions.push("\treturn\n");
        L2_instructions.push("\trax <- "+root->value->to_string()+"\n");
        return true;
      }
      //unmatched.push_back(root);
      return false;

    }
  };

  struct Tile_call : Tile {
    Tile_call(){
      cost = -1; // Dependent on number of arguments
      size = -1; // Dependent on number of arguments
    }

    virtual bool match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions, std::string longest_label_name, int64_t &label_count){
      cout << "in tile call\n";
      bool is_call = root->operand_type == Instruction_type::CALL;
      if (is_call) {
        L2_instructions.push("\t"+longest_label_name+std::to_string(label_count)+"\n");
        L2_instructions.push("\tcall " + root->value->to_string() + " " + to_string(root->children.size()) + "\n");
        for (int i=0; i<root->children.size(); i++) {
          Node* child = root->children[i];
          if (i >= argument_registers.size()) {
            int stack_loc = -16 - 8 * (i - argument_registers.size());
            L2_instructions.push("\tmem rsp " + to_string(stack_loc) + " <- " + child->value->to_string() + "\n");
          } else {
            string arg_register = argument_registers[i];
            L2_instructions.push("\t" + argument_registers[i] + " <- " + child->value->to_string() + "\n");
          }
          if (!child->children.empty()) {
            unmatched.push_back(child);
          }
        }
        L2_instructions.push("\tmem rsp -8 <- "+longest_label_name+std::to_string(label_count)+"\n");
        label_count++;
        return true;
      }

      //unmatched.push_back(root);
      return false;
    }
  };

  struct Tile_call_store : Tile {
    Tile_call_store(){
      cost = -1; // Dependent on number of arguments
      size = -1; // Dependent on number of arguments
    }

    virtual bool match(Node* root, std::vector<Node*> &unmatched, std::stack<std::string> &L2_instructions, std::string longest_label_name, int64_t &label_count){
      cout << "in tile call store\n";
      bool is_call_store = root->operand_type == Instruction_type::CALLSTORE;
      if (is_call_store) {
        L2_instructions.push("\t" + root->value->to_string() + " <- rax");
        L2_instructions.push("\t"+longest_label_name+std::to_string(label_count)+"\n");
        L2_instructions.push("\tcall " + root->value->to_string() + " " + to_string(root->children.size()) + "\n");
        for (int i=0; i<root->children.size(); i++) {
          Node* child = root->children[i];
          if (i >= argument_registers.size()) {
            int stack_loc = -16 - 8 * (i - argument_registers.size());
            L2_instructions.push("\tmem rsp " + to_string(stack_loc) + " <- " + child->value->to_string());
          } else {
            string arg_register = argument_registers[i];
            L2_instructions.push("\t" + argument_registers[i] + " <- " + child->value->to_string());
          }
          if (!child->children.empty()) {
            unmatched.push_back(child);
          }
        }
        L2_instructions.push("\tmem rsp -8 <- "+longest_label_name+std::to_string(label_count)+"\n");
        label_count++;
        return true;
      }

      //unmatched.push_back(root);
      return false;
    }
  };
}
