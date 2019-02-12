#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <stack>

#include <graph_coloring.h>
#include <transformer.h>
#include <utils.h>
#include <functional>

using namespace std;
using L2::Node;
using L2::registers;

namespace L2{

  map<string, Node> color_registers() {
    map<string, Node> new_graph;
    for (string r: registers) {
      Node n;
      n.name = r;
      n.color = r;
      n.edges.insert(n.edges.end(), registers.begin(), registers.end());
      n.edges.erase(remove(n.edges.begin(), n.edges.end(), r), n.edges.end());
      new_graph.insert(pair<string, Node>(r, n));
    }
    return new_graph;
  }

  void remove_node(Node top, map<string, Node> &graph) {
    for(map<string, Node>::iterator it = graph.begin(); it != graph.end(); it++){
      it->second.edges.erase(remove(it->second.edges.begin(), it->second.edges.end(), top.name), it->second.edges.end());
    }
    graph.erase(top.name);
  }

  stack<Node> generate_stack(map<string, Node> graph) {
    stack<Node> s;
    while (graph.size() > 15){
      vector<Node> tops = {};
      int curr_max = 0;
      for(map<string, Node>::iterator it = graph.begin(); it != graph.end(); it++) {
        if (find(begin(registers), end(registers), it->first) != end(registers)){
          continue;
        }
        if(it->second.edges.size() > curr_max){
          tops = {it->second};
          curr_max = it->second.edges.size();
        } else if (it->second.edges.size() == curr_max){
          tops.push_back(it->second);
        }
      }

      for (Node top : tops){
        s.push(top);
        remove_node(top, graph);
      }
    }
    return s;
  }

  vector<string> get_color_conflicts(string name, map<string, Node> graph) {
    vector<string> conflicts;
    Node n = graph[name];
    for (string edge: n.edges) {
      string color = graph[edge].color;
      conflicts.push_back(color);
    }
    return conflicts;
  }

  void add_node(map<string, Node> &new_g, Node node, vector<string> edges) {
    new_g.insert(pair<string,Node>(node.name, node));
    for(map<string, Node>::iterator it = new_g.begin(); it != new_g.end(); it++) {
      if (find(begin(edges), end(edges), it->second.name) != end(edges)){
        if (find(begin(new_g[it->second.name].edges), end(new_g[it->second.name].edges), node.name) == end(new_g[it->second.name].edges)){
          new_g[it->second.name].edges.push_back(node.name);
        }
        new_g[node.name].edges.push_back(it->second.name);
      }
    }
  }

  void assign_color(string name, map<string, Node> &new_g, vector<string> &to_spill) {
    vector<string> conflicts = get_color_conflicts(name, new_g);
    for (string color: registers) {
      if (find(begin(conflicts), end(conflicts), color) == end(conflicts)) {
        new_g[name].color = color;
        return;
      }
    }
    to_spill.push_back(name);
  }

  bool assign_colors(map<string, Node> &g, vector<string> &to_spill) {
    stack<Node> stack = generate_stack(g);
    map<string, Node> new_g = color_registers();
    while (!stack.empty()) {
      Node n = stack.top();
      stack.pop();

      Node new_n;
      new_n.name = n.name;
      add_node(new_g, new_n, g[new_n.name].edges);
      assign_color(new_n.name, new_g, to_spill);
    }
    g = new_g;
    return !to_spill.empty();
  }
}
