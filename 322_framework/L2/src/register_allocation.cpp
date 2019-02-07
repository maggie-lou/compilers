#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <stack>

#include <register_allocation.h>
#include <graph_coloring.h>
#include <transformer.h>
#include <utils.h>
#include <functional>

using namespace std;
using L2::Node;
using L2::registers;
using L2::assign_colors;

namespace L2{

  void allocate_registers(Program p) {
    bool spill;
    do {
     map<string, Node> interference_graph = generate_graph(p);
     vector<string> to_spill;
     spill = assign_colors(interference_graph, to_spill);
     if (spill) {
      for (string var: to_spill) {
          spill(p, var, "S");
        }
     }
    } while (spill);
  }

}
