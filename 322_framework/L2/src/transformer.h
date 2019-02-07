#pragma once

#include <L2.h>

using namespace std;

namespace L2{

  struct Node {
    string name;
    vector<string> edges;
  };

  map<string, Node> generate_graph(Program p);
  void generate_and_print_graph(Program p);

}
