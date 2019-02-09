#pragma once

#include <L2.h>

using namespace std;

namespace L2{

  struct Node {
    string name;
    string color;
    vector<string> edges;
  };

  map<string, Node> generate_graph(Function* f);
  void generate_and_print_graph(Program p);

}
