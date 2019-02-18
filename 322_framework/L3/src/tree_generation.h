#pragma once

#include <L3.h>
#include <unordered_map>

using namespace std;

namespace L3{
vector<Node*> generate_and_merge_trees_all(vector<vector<Instruction*>> contexts,
                                           vector<vector<string>> in,
                                           vector<vector<string>> out,
                                           unordered_map<string, string> label_map);

}
