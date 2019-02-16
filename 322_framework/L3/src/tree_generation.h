#pragma once

#include <L3.h>

using namespace std;

namespace L3{

  struct Tree_node : Node {
    vector<Tree_node*> children;
  };

}
