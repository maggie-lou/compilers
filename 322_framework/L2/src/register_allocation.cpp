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
      cout << "replacement: " << replacement_reg << "\n";
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
        cout << "g name of register: " << g->item_to_string() << "\n";
      }

      for (Item*& k : kill) {
        replace_var_with_register(k, interference_graph);
        cout << "k name of register: " << k->item_to_string() << "\n";
      }
    }
  }

  void allocate_registers(Function* &f) {
    bool spill;
    map<string, Node> interference_graph;
    do {
      spill = false;
      cout << "start generating interference graph...\n";
      interference_graph = L2::generate_graph(f);
      cout << "finished generating interference graph...\n";
      vector<string> to_spill;
      cout << "start assigning colors...\n";
      spill = L2::assign_colors(interference_graph, to_spill);
      cout << "finished assigning colors...\n";
      cout << "start spilling...\n";
      if (spill) {
        for (string var: to_spill) {
          cout << "spilling " << var << "\n";
          f = L2::spill(f, var, "S"); // TODO find prefix?
        }
      }
    } while (spill);
    cout << "finished spilling...\n";
    cout << "start replacing...\n";
    replace_vars_with_registers(f, interference_graph);
  }
}
