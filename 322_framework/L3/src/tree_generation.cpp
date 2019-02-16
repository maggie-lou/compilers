// merging trees
// make a list of all Roots
// go through each tree to see if one of the Roots shows up as a child
// if so, do some checks
// - liveness analysis, only merge if Root will be dead
// - ...
// if safe, merge the two trees

//
// vector<Tree_node*> merge_tree(vector<Tree_node*>){
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

using namespace std;
using namespace L3;

namespace L3{

  Tree_node* generate_node(Item* i) {
    Tree_node *t = new Tree_node();
    t->value = i;
    return t;
  }

  void add_child(Tree_node* t, Item* child, map<string, vector<Tree_node*>> &leaf_map) {
    t->children.push_back(generate_node(child));
    if (auto variable = dynamic_cast<Variable*>(child)) {
      vector<Tree_node*> trees_containing_leaf;
      if (leaf_map.count(variable->name)) {
        trees_containing_leaf = leaf_map[variable->name];
      }
      trees_containing_leaf.push_back(t);
      leaf_map.insert(pair<string, vector<Tree_node*>>(variable->name, trees_containing_leaf));
    }
  }

  Tree_node* generate_tree(Instruction* i, map<string, vector<Tree_node*>> &leaf_map) {
    Tree_node* t;

    if (auto i_cast = dynamic_cast<Instruction_assign*>(i)) {
      t = generate_node(i_cast->dest);
      t->operand_type = i_cast->type;
      add_child(t, i_cast->source, leaf_map);
    } else if (auto i_cast = dynamic_cast<Instruction_op*>(i)) {
      t = generate_node(i_cast->dest);
      t->operand_type = i_cast->type;
      t->op_name = i_cast->op;
      add_child(t, i_cast->t2, leaf_map);
      add_child(t, i_cast->t1, leaf_map);
    } else if (auto i_cast = dynamic_cast<Instruction_cmp*>(i)) {
      t = generate_node(i_cast->dest);
      t->operand_type = i_cast->type;
      t->op_name = i_cast->cmp;
      add_child(t, i_cast->t1, leaf_map);
      add_child(t, i_cast->t2, leaf_map);
    } else if (auto i_cast = dynamic_cast<Instruction_load*>(i)) {
      t = generate_node(i_cast->dest);
      t->operand_type = i_cast->type;
      add_child(t, i_cast->source, leaf_map);
    } else if (auto i_cast = dynamic_cast<Instruction_store*>(i)) {
      t = generate_node(i_cast->dest);
      t->operand_type = i_cast->type;
      add_child(t, i_cast->source, leaf_map);
    } else if (auto i_cast = dynamic_cast<Instruction_goto*>(i)) {
      t = generate_node(i_cast->label);
      t->operand_type = i_cast->type;
    } else if (auto i_cast = dynamic_cast<Instruction_label*>(i)) {
      t = generate_node(i_cast->label);
      t->operand_type = i_cast->type;
    } else if (auto i_cast = dynamic_cast<Instruction_jump*>(i)) {
      t = generate_node(i_cast->var);
      t->operand_type = i_cast->type;
      add_child(t, i_cast->label, leaf_map);
    } else if (auto i_cast = dynamic_cast<Instruction_ret_void*>(i)) {
      t->operand_type = i_cast->type;
    } else if (auto i_cast = dynamic_cast<Instruction_ret*>(i)) {
      t = generate_node(i_cast->t);
      t->operand_type = i_cast->type;
    } else if (auto i_cast = dynamic_cast<Instruction_call*>(i)) {
      t = generate_node(i_cast->callee);
      t->operand_type = i_cast->type;
      for (Item* arg: i_cast->args) {
        add_child(t, arg, leaf_map);
      }
    } else if (auto i_cast = dynamic_cast<Instruction_call_store*>(i)) {
      t = generate_node(i_cast->dest);
      t->operand_type = i_cast->type;

      add_child(t, i_cast->callee, leaf_map);
      for (Item* arg: i_cast->args) {
        add_child(t, arg, leaf_map);
      }
    }

    return t;
  }

  void merge(Tree_node *t, Tree_node *merge_into, vector<Tree_node*> &trees, int t_index) {
    string merge_name = t->value->to_string();
    for (int i=0; i < merge_into->children.size(); i++) {
      Tree_node *child = merge_into->children[i];
      if(child->value->type == Item_type::VARIABLE) {
        string child_name = child->value->to_string();
        if (merge_name == child_name) {
          merge_into->children[i] = t;
        }
      }
    }
    trees.erase(trees.begin() + t_index);
  }

  vector<Tree_node*> generate_and_merge_trees(vector<Instruction*> context, vector<vector<string>> in_sets, vector<vector<string>> out_sets){
    vector<Tree_node*> trees;
    map<string, vector<Tree_node*>> leaf_map;
    for (Instruction* i: context) {
      Tree_node* tree = generate_tree(i, leaf_map);
      trees.push_back(tree);
    }

    for (int i=0; i<trees.size(); i++) {
      if (trees[i]->value->type == Item_type::VARIABLE) {
        string var_name = trees[i]->value->to_string();
        for (int j=i+1; j<trees.size(); j++) {
          vector<string> out_set = out_sets[j];
          vector<string> in_set = in_sets[j];
          if (contains(out_set, var_name)) {
            break;
          }
          if (contains(in_set, var_name)) {
            merge(trees[i], trees[j], trees, i);
            break;
          }
        }
      }
    }

    return trees;
  }
}
