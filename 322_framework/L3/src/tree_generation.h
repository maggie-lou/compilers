#pragma once

#include <L3.h>
#include <unordered_map>

using namespace std;

namespace L3{
vector<Node*> generate_and_merge_trees(vector<Instruction*> context,
                                       vector<vector<string>> in_sets,
                                       vector<vector<string>> out_sets,
                                       unordered_map<string, string> label_map);

}
