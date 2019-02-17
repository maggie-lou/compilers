// merging trees
// make a list of all Roots
// go through each tree to see if one of the Roots shows up as a child
// if so, do some checks
// - liveness analysis, only merge if Root will be dead
// - ...
// if safe, merge the two trees

//
// vector<Node*> merge_tree(vector<Node*>){
//   // go through each tree and look at the Root
//   // check the map, and merge trees
//   // delete merged tree
// }
//
// bool check_liveness(Variable v, vector<string> out_set){
//
// }
#include <L3.h>
#include <tree_generation.h>

#include <vector>
#include <iostream>
#include <string>
#include <iterator>
#include <unordered_map>
#include <utils.h>

using namespace std;
using namespace L3;

namespace L3{

  Node* generate_node(Item* i, unordered_map<string, string> label_map) {
    Node *t = new Node();
    if (Label* label = dynamic_cast<Label*>(i)){
      label->name = label_map[label->name];
      t->value = label;
    } else {
      t->value = i;
    }
    return t;
  }

  void add_child(Node* t, Item* child, map<string, vector<Node*>> &leaf_map, unordered_map<string, string> label_map) {
    t->children.push_back(generate_node(child, label_map));
    if (auto variable = dynamic_cast<Variable*>(child)) {
      vector<Node*> trees_containing_leaf;
      if (leaf_map.count(variable->name)) {
        trees_containing_leaf = leaf_map[variable->name];
      }
      trees_containing_leaf.push_back(t);
      leaf_map.insert(pair<string, vector<Node*>>(variable->name, trees_containing_leaf));
    }
  }

  Node* generate_tree(Instruction* i, map<string, vector<Node*>> &leaf_map, unordered_map<string, string> label_map) {
    Node* t;

    if (auto i_cast = dynamic_cast<Instruction_assign*>(i)) {
      t = generate_node(i_cast->dest, label_map);
      t->operand_type = i_cast->type;
      add_child(t, i_cast->source, leaf_map, label_map);
    } else if (auto i_cast = dynamic_cast<Instruction_op*>(i)) {
      t = generate_node(i_cast->dest, label_map);
      t->operand_type = i_cast->type;
      t->op_name = i_cast->op;
      add_child(t, i_cast->t1, leaf_map, label_map);
      add_child(t, i_cast->t2, leaf_map, label_map);
    } else if (auto i_cast = dynamic_cast<Instruction_cmp*>(i)) {
      t = generate_node(i_cast->dest, label_map);
      t->operand_type = i_cast->type;
      t->op_name = i_cast->cmp;
      add_child(t, i_cast->t1, leaf_map, label_map);
      add_child(t, i_cast->t2, leaf_map, label_map);
    } else if (auto i_cast = dynamic_cast<Instruction_load*>(i)) {
      t = generate_node(i_cast->dest, label_map);
      t->operand_type = i_cast->type;
      add_child(t, i_cast->source, leaf_map, label_map);
    } else if (auto i_cast = dynamic_cast<Instruction_store*>(i)) {
      t = generate_node(i_cast->dest, label_map);
      t->operand_type = i_cast->type;
      add_child(t, i_cast->source, leaf_map, label_map);
    } else if (auto i_cast = dynamic_cast<Instruction_goto*>(i)) {
      t = generate_node(i_cast->label, label_map);
      t->operand_type = i_cast->type;
    } else if (auto i_cast = dynamic_cast<Instruction_label*>(i)) {
      t = generate_node(i_cast->label, label_map);
      t->operand_type = i_cast->type;
    } else if (auto i_cast = dynamic_cast<Instruction_jump*>(i)) {
      t = generate_node(i_cast->var, label_map);
      t->operand_type = i_cast->type;
      add_child(t, i_cast->label, leaf_map, label_map);
    } else if (auto i_cast = dynamic_cast<Instruction_ret_void*>(i)) {
      t->operand_type = i_cast->type;
    } else if (auto i_cast = dynamic_cast<Instruction_ret*>(i)) {
      t = generate_node(i_cast->t, label_map);
      t->operand_type = i_cast->type;
    } else if (auto i_cast = dynamic_cast<Instruction_call*>(i)) {
      t = generate_node(i_cast->callee, label_map);
      t->operand_type = i_cast->type;
      for (Item* arg: i_cast->args) {
        add_child(t, arg, leaf_map, label_map);
      }
    } else if (auto i_cast = dynamic_cast<Instruction_call_store*>(i)) {
      t = generate_node(i_cast->dest, label_map);
      t->operand_type = i_cast->type;

      add_child(t, i_cast->callee, leaf_map, label_map);
      for (Item* arg: i_cast->args) {
        add_child(t, arg, leaf_map, label_map);
      }
    }

    return t;
  }

  void merge(Node *t, Node *merge_into, vector<Node*> &trees, int t_index) {
    if(merge_into->operand_type==Instruction_type::RETVOID||merge_into->operand_type==Instruction_type::RET||merge_into->operand_type==Instruction_type::CALL){
      return;
    }
    string merge_name = t->value->to_string();
    for (int i=0; i < merge_into->children.size(); i++) {
      Node *child = merge_into->children[i];
      if(child->value->type == Item_type::VARIABLE) {
        string child_name = child->value->to_string();
        if (merge_name == child_name) {
          merge_into->children[i] = t;
        }
      }
    }
    trees.erase(trees.begin() + t_index);
  }

  vector<Node*> generate_and_merge_trees(vector<Instruction*> context, vector<vector<string>> in_sets, vector<vector<string>> out_sets, unordered_map<string, string> label_map){
    vector<Node*> trees;
    map<string, vector<Node*>> leaf_map;
    for (Instruction* i: context) {
      Node* tree = generate_tree(i, leaf_map, label_map);
      trees.push_back(tree);
    }

    for (int i=0; i<trees.size(); i++) {
      if (trees[i]->value->type == Item_type::VARIABLE) {
        string var_name = trees[i]->value->to_string();
        for (int j=i+1; j<trees.size(); j++) {
          vector<string> out_set = out_sets[j];
          vector<string> in_set = in_sets[j];
          if (L3::contains(out_set, var_name)) {
            break;
          }
          if (L3::contains(in_set, var_name)) {
            merge(trees[i], trees[j], trees, i);
            break;
          }
        }
      }
    }

    return trees;
  }
}
