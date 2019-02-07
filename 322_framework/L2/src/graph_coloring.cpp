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

  void color_registers(vector<L2::Node> &graph) {
    for (L2::Node n: graph) {
      if (find(begin(registers), end(registers), n.name) != end(registers)) {
          n.color = n.name;
      }
    }
  }

  vector<L2::Node> generate_graph_vector(map<string, L2::Node> g) {
    vector<L2::Node> new_g;
    for(map<string,Node>::iterator iter = g.begin(); iter != g.end(); ++iter) {
      new_g.push_back(iter->second);
    }
    return new_g;
  }

  void sort_graph(vector<Node> &graph) {
    sort( graph.begin(), graph.end(), [](const L2::Node&lhs, const L2::Node& rhs) {
        return lhs.edges.size() > rhs.edges.size(); // Descending order
    });

    while (graph[0].edges.size() >= registers.size()) {
      graph.push_back(graph[0]);
      graph.erase(graph.begin());
    }
  }

  void remove_node(string name, vector<Node> graph) {
    for (Node n: graph) {
      n.edges.erase(remove(n.edges.begin(), n.edges.end(), name), n.edges.end());
    }
  }

  stack<Node> generate_stack(vector<Node> graph) {
    stack<Node> s;
    while (!graph.empty()) {
      Node top = graph[0];
      graph.erase(graph.begin());
      s.push(top);
      remove_node(top.name, graph);
    }
    return s;
  }

  vector<string> get_color_conflicts(Node n, map<string, Node> graph) {
    vector<string> conflicts;

    for (string edge: n.edges) {
      string color = graph[edge].color;
      conflicts.push_back(color);
    }

    return conflicts;
  }

  void add_edges(map<string, Node> &graph, string node, vector<string> edges) {
    for (string edge: edges) {
      graph[edge].edges.push_back(node);
    }
  }

  void assign_color(Node n, map<string, Node> &graph, vector<string> &to_spill) {
    vector<string> conflicts = get_color_conflicts(n, graph);

    for (string color: registers) {
      if (find(begin(conflicts), end(conflicts), color) == end(conflicts)) {
        n.color = color;
        graph.insert(pair<string,Node>(n.name, n));
        add_edges(graph, n.name, n.edges);
        return;
      }
    }

    to_spill.push_back(n.name);
  }

  bool assign_colors(map<string, L2::Node> g, vector<string> &to_spill) {
    vector<L2::Node> graph = generate_graph_vector(g);

    color_registers(graph);
    sort_graph(graph);
    stack<Node> stack = generate_stack(graph);

    map<string, Node> colored_graph;
    while (!stack.empty()) {
      Node n = stack.top();
      assign_color(n, colored_graph, to_spill);
    }

    return !to_spill.empty();
  }
}

