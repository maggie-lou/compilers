// Tile_node
// type
// operand
// children
// associated instruction
// cost
// size
// other constraints

// sort tiles by size & cost

// tile & tree matching
// recursion
// helper function that takes in two Nodes and compare type & operand
// call the helper function on children

// maximal munch
// start at root
// start from the largest tile and try to match
// try to match subtrees
//
// vector<Instruction*> maximal_munching(Tree_node* n){
//
// }
#include <utils.h>
#include <vector>
#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>
#include <L3.h>
#include <tile_matching.h>
#include <tree_generation.h>
#include <unordered_map>

using namespace std;

namespace L3{

  void append(stack<string> &s1, stack<string> s2) {
    stack<string> temp;
    while (!s2.empty()) {
      temp.push(s2.top());
      s2.pop();
    }
    while (!temp.empty()) {
      s1.push(temp.top());
      s2.pop();
    }
  }

  vector<Tile> generate_tiles() {
    vector<Tile> tiles;
    tiles.push_back(Tile_assign());
    tiles.push_back(Tile_op());
    tiles.push_back(Tile_cmp());
    tiles.push_back(Tile_load());
    tiles.push_back(Tile_store());
    tiles.push_back(Tile_goto());
    tiles.push_back(Tile_label());
    tiles.push_back(Tile_jump());
    tiles.push_back(Tile_ret_void());
    tiles.push_back(Tile_ret());
    tiles.push_back(Tile_call());
    tiles.push_back(Tile_call_store());

    // Sort tiles ascending order
    // sort by cost, and then size
    sort(tiles.begin(), tiles.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.cost == rhs.cost) {
        return lhs.size > rhs.size;
        } else {
        return lhs.cost < rhs.cost;
        }
    });

    return tiles;
  }

  stack<string> generate_l2_instructions(vector<Node*> trees, std::string longest_label_name, int64_t &label_count) {
    stack<string> L2_instructions;
    if (trees.empty()) {
      return L2_instructions;
    }

    vector<Tile> tiles = generate_tiles();
    for (Node* tree : trees) {
      stack<string> generated_instructions;
      vector<Node*> unmatched;

      for (Tile tile : tiles) {
        if (tile.match(tree, unmatched, generated_instructions, longest_label_name, label_count)) {
          stack<string> child_instructions = generate_l2_instructions(unmatched, longest_label_name, label_count);
          append(L2_instructions, child_instructions);
          break;
        }
      }
    }

    return L2_instructions;
  }
}
