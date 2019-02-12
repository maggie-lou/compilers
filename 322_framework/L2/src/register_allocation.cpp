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
#include <spiller.h>
#include <functional>

using namespace std;
using L2::Node;
using L2::registers;

namespace L2{

  void replace_var_with_register(Item* &i, map<string, Node> graph) {
    if (Var_item* var = dynamic_cast<Var_item*>(i)){
      string replacement_reg = graph[var->var_name].color;
      Register_item* r = new Register_item(replacement_reg);
      i = r;
    }
  }

  void replace_vars_with_registers(Function* &f, map<string, Node> interference_graph) {
    for (Instruction* i : f->instructions) {
      vector<reference_wrapper<Item*>> gen = i->generate_gen();
      vector<reference_wrapper<Item*>> kill = i->generate_kill();

      for (Item*& g : gen) {
        replace_var_with_register(g, interference_graph);
      }

      for (Item*& k : kill) {
        replace_var_with_register(k, interference_graph);
      }
    }
  }

  void allocate_registers(Function* &f) {
    bool spill;
    map<string, Node> interference_graph;
    do {
      spill = false;
      interference_graph = L2::generate_graph(f);
      vector<string> to_spill;
      spill = L2::assign_colors(interference_graph, to_spill);
      if (spill) {
        for (string var: to_spill) {
          f = L2::spill(f, var, var+"_ENd"); // TODO find prefix?
        }
      }
    } while (spill);
    replace_vars_with_registers(f, interference_graph);
  }
}
