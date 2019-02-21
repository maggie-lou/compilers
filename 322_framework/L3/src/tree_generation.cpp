#include <L3.h>
#include <tree_generation.h>

#include <vector>
#include <iostream>
#include <string>
#include <iterator>
#include <unordered_map>
#include <utils.h>
#include <set>

using namespace std;
using namespace L3;

namespace L3{
  Node* generate_node(Item* i, unordered_map<string, string> label_map, bool translate) {
    Node *t = new Node();
    if (translate){
      if (Label* label = dynamic_cast<Label*>(i)){
        label->name = label_map[label->name];
        t->value = label;
        return t;
      }
    }
    t->value = i;
    return t;
  }

  void add_child(Node* t, Item* child, map<string, vector<Node*>> &leaf_map, unordered_map<string, string> label_map, bool translate) {
    t->children.push_back(generate_node(child, label_map, translate));
    if (auto variable = dynamic_cast<Variable*>(child)) {
      vector<Node*> trees_containing_leaf;
      if (leaf_map.count(variable->name)) {
        trees_containing_leaf = leaf_map[variable->name];
      }
      trees_containing_leaf.push_back(t);
      leaf_map.insert(pair<string, vector<Node*>>(variable->name, trees_containing_leaf));
    }
  }

  Node* generate_tree(Instruction* i, map<string, vector<Node*>> &leaf_map, unordered_map<string, string> &label_map) {
    Node* t;

    if (auto i_cast = dynamic_cast<Instruction_assign*>(i)) {
      t = generate_node(i_cast->dest, label_map, false);
      t->operand_type = i_cast->type;
      add_child(t, i_cast->source, leaf_map, label_map, false);
    } else if (auto i_cast = dynamic_cast<Instruction_op*>(i)) {
      t = generate_node(i_cast->dest, label_map, false);
      t->operand_type = i_cast->type;
      t->op_name = i_cast->op;
      if (auto dest1 = dynamic_cast<Variable*>(i_cast->dest)){
        if (auto v2 = dynamic_cast<Variable*>(i_cast->t2)){
          if (dest1->name == v2->name){
            Item* temp = i_cast->t1;
            i_cast->t1 = i_cast->t2;
            i_cast->t2 = temp;
          }
        }
      }

      add_child(t, i_cast->t1, leaf_map, label_map, false);
      add_child(t, i_cast->t2, leaf_map, label_map, false);
    } else if (auto i_cast = dynamic_cast<Instruction_cmp*>(i)) {
      t = generate_node(i_cast->dest, label_map, false);
      t->operand_type = i_cast->type;
      t->op_name = i_cast->cmp;
      add_child(t, i_cast->t1, leaf_map, label_map, false);
      add_child(t, i_cast->t2, leaf_map, label_map, false);
    } else if (auto i_cast = dynamic_cast<Instruction_load*>(i)) {
      t = generate_node(i_cast->dest, label_map, false);
      t->operand_type = i_cast->type;
      add_child(t, i_cast->source, leaf_map, label_map, false);
    } else if (auto i_cast = dynamic_cast<Instruction_store*>(i)) {
      t = generate_node(i_cast->dest, label_map, false);
      t->operand_type = i_cast->type;
      add_child(t, i_cast->source, leaf_map, label_map, false);
    } else if (auto i_cast = dynamic_cast<Instruction_goto*>(i)) {
      t = generate_node(i_cast->label, label_map, true);
      t->operand_type = i_cast->type;
    } else if (auto i_cast = dynamic_cast<Instruction_label*>(i)) {
      t = generate_node(i_cast->label, label_map, true);
      t->operand_type = i_cast->type;
    } else if (auto i_cast = dynamic_cast<Instruction_jump*>(i)) {
      t = generate_node(i_cast->var, label_map, true);
      t->operand_type = i_cast->type;
      add_child(t, i_cast->label, leaf_map, label_map, true);
    } else if (auto i_cast = dynamic_cast<Instruction_ret_void*>(i)) {
      t = new Node();
      t->operand_type = i_cast->type;
    } else if (auto i_cast = dynamic_cast<Instruction_ret*>(i)) {
      t = generate_node(i_cast->t, label_map, false);
      t->operand_type = i_cast->type;
    } else if (auto i_cast = dynamic_cast<Instruction_call*>(i)) {
      t = generate_node(i_cast->callee, label_map, false);
      t->operand_type = i_cast->type;
      for (Item* arg: i_cast->args) {
        add_child(t, arg, leaf_map, label_map, false);
      }
    } else if (auto i_cast = dynamic_cast<Instruction_call_store*>(i)) {
      t = generate_node(i_cast->dest, label_map, false);
      t->operand_type = i_cast->type;

      add_child(t, i_cast->callee, leaf_map, label_map, false);
      for (Item* arg: i_cast->args) {
        add_child(t, arg, leaf_map, label_map, false);
      }
    }

    return t;
  }

  void merge(Node *t, Node *merge_into, vector<Node*> &trees, int t_index) {
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

  bool is_leaf(Node* tree,  string var_name){
    if (tree->children.empty()){
      if (tree->value->to_string() == var_name) return true;
      return false;
    }
    bool child_leaf = false;
    for (Node* child : tree->children){
      child_leaf |= is_leaf(child, var_name);
    }
    return child_leaf;
  }

  vector<Node*> generate_and_merge_trees(vector<Instruction*> context, vector<vector<string>> in_sets, vector<vector<string>> out_sets, unordered_map<string, string> label_map){
    vector<Node*> trees;
    map<string, vector<Node*>> leaf_map;
    for (Instruction* i: context) {
      Node* tree = generate_tree(i, leaf_map, label_map);
      trees.push_back(tree);
    }

    for (int i=0; i<trees.size(); i++) {
      if (trees[i]->operand_type==Instruction_type::RETVOID||trees[i]->operand_type==Instruction_type::RET||trees[i]->operand_type==Instruction_type::CALL||trees[i]->operand_type==Instruction_type::CALLSTORE){
        continue;
      }
      vector<string> in_set_i = in_sets[i];
      if (trees[i]->value->type == Item_type::VARIABLE) {
        string var_name = trees[i]->value->to_string();
        for (int j=i+1; j<trees.size(); j++) {

          vector<string> out_set = out_sets[j];
          vector<string> in_set = in_sets[j];
          if (L3::contains(in_set, var_name)) {
            //  don't merge if root is used by other instruction
            break;
          }

          bool do_not_merge = false;
          for (string out_var : out_set){
            // don't merge if var in IN was changed by other instructions
            if (L3::contains(in_set_i, out_var)){
              do_not_merge = true;
              break;
            }
          }
          if (do_not_merge){
            break;
          }

          if (trees[j]->operand_type==Instruction_type::RETVOID||trees[j]->operand_type==Instruction_type::RET||trees[j]->operand_type==Instruction_type::CALL||trees[j]->operand_type==Instruction_type::CALLSTORE){
            continue;
          }

          if (is_leaf(trees[j], var_name)) {
            merge(trees[i], trees[j], trees, i);
            break;
          }
        }
      }
    }
    return trees;
  }

  vector<Node*> generate_and_merge_trees_all(vector<vector<Instruction*>> contexts, vector<vector<string>> in, vector<vector<string>> out, unordered_map<string, string> label_map) {
    vector<Node*> all_trees;
    for (auto context : contexts){
      auto trees = generate_and_merge_trees(context, in, out, label_map);
      all_trees.insert(all_trees.end(), trees.begin(), trees.end());
    }
    return all_trees;
  }

}
