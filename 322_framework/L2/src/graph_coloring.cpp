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

  map<string, Node> color_registers(map<string, Node> graph) {
    map<string, Node> colored_graph;
    for (string r: registers) {
      Node n = graph[r];
      n.color = r;
      colored_graph.insert(pair<string,Node>(n.name, n));
    }
    return colored_graph;
  }

  vector<L2::Node> generate_graph_vector(map<string, L2::Node> g) {
    vector<L2::Node> new_g;
    for(map<string,Node>::iterator iter = g.begin(); iter != g.end(); ++iter) {
      if (find(begin(registers), end(registers), iter->second.name) == end(registers)) {
        new_g.push_back(iter->second);
      }
    }
    return new_g;
  }

  void sort_graph(vector<Node> &graph) {
    if (!graph.empty()){
      sort( graph.begin(), graph.end(), [](const L2::Node&lhs, const L2::Node& rhs) {
          return lhs.edges.size() > rhs.edges.size(); // Descending order
      });

      int iterations  = graph.size();
      while (iterations > 0 && graph[0].edges.size() >= registers.size()) {
        graph.push_back(graph[0]);
        graph.erase(graph.begin());
	iterations--;
      }
    }
  }

  void remove_node(string name, vector<Node> &graph) {
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
    for(map<string, Node>::iterator it = graph.begin(); it != graph.end(); it++) {
      Node n = it->second;
      if (find(begin(edges), end(edges), n.name) != end(edges)){
        graph[n.name].edges.push_back(node);
      }
    }
  }

  void assign_color(Node &n, map<string, Node> &graph, vector<string> &to_spill) {
    // cout << "name: " << n.name << "\n";
    // cout << "old color: " << n.color << "\n";
    vector<string> conflicts = get_color_conflicts(n, graph);
    // cout << "conflicts: ";
    // L2::print_vector(conflicts);
    // cout << endl;

    for (string color: registers) {
      if (find(begin(conflicts), end(conflicts), color) == end(conflicts)) {
        // cout << "Assigning color "<< color << endl;
        n.color = color;
        // if (graph.count(n.name)) {
        //   cout << "already in graph" << endl;
        //   auto ex = graph[n.name];
        //   cout << "WTF " << ex.name << " color " << ex.color << endl;
        // }
        // cout << "before insertion size: " << graph.size() << endl;
        graph.insert(pair<string,Node>(n.name, n));
        // cout << "after insertion size: " << graph.size() << endl;
        // for(map<string, Node>::iterator it = graph.begin(); it != graph.end(); it++) {
        //   Node n = it->second;
        //   cout << n.name << endl;
        //   cout << "Color: " << n.color << endl;
        // }
        add_edges(graph, n.name, n.edges);
        return;
      }
    }
    // cout << "didn't find available color\n";
    to_spill.push_back(n.name);
  }

  bool assign_colors(map<string, Node> &g, vector<string> &to_spill) {
    // cout << "generating graph vector...\n";
    vector<L2::Node> graph_vector = generate_graph_vector(g);
    // for (L2::Node nn : graph_vector){
    //   cout << "Name: " << nn.name << "\n";
    // }
    // cout << "sorting...\n";
    sort_graph(graph_vector);
    // cout << "generating stack...\n";
    stack<Node> stack = generate_stack(graph_vector);

    // cout << "coloring regs...\n";
    // cout << "In graph" << endl;
    map<string, Node> colored_graph = color_registers(g);
    while (!stack.empty()) {
      Node n = stack.top();
      if (find(begin(registers), end(registers), n.name) != end(registers)) {
        continue;
      }
      stack.pop();
      // cout << "in assign_colors, name: " << n.name << "...\n";
      assign_color(n, colored_graph, to_spill);
      //cout << "assigned node "<< n.name << " color " << colored_graph[n.name].color << "...\n";
    }
    g = colored_graph;

    return !to_spill.empty();
  }
}
