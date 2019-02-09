#pragma once

#include <L2.h>
#include <transformer.h>

using namespace std;
using L2::Node;

namespace L2{
  bool assign_colors(map<string,Node> &graph, vector<string> &to_spill);
}
