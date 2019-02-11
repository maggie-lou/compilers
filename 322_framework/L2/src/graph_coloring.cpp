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

  // vector<Node> generate_graph_vector(map<string, Node> g) {
  //   vector<Node> new_g;
  //   for(map<string,Node>::iterator iter = g.begin(); iter != g.end(); ++iter) {
  //     if (find(begin(registers), end(registers), iter->second.name) == end(registers)) {
  //       new_g.push_back(iter->second);
  //     }
  //   }
  //   return new_g;
  // }
  //
  // void sort_graph(vector<Node> &graph) {
  //   if (!graph.empty()){
  //     sort( graph.begin(), graph.end(), [](const L2::Node&lhs, const L2::Node& rhs) {
  //         return lhs.edges.size() < rhs.edges.size(); // Descending order
  //     });
  //
  //     int iterations  = graph.size();
  //     while (iterations > 0 && graph[0].edges.size() >= registers.size()) {
  //       graph.push_back(graph[0]);
  //       graph.erase(graph.begin());
	//       iterations--;
  //     }
  //   }
  // }

  void remove_node(Node top, map<string, Node> &graph) {
    for(map<string, Node>::iterator it = graph.begin(); it != graph.end(); it++){
      // Node n = it->second;
      // cout << n.name << " old edges: \n";
      // L2::print_vector(n.edges);
      // cout << "\n";
      it->second.edges.erase(remove(it->second.edges.begin(), it->second.edges.end(), top.name), it->second.edges.end());
      // it->second = n;
      // cout << n.name << " new edges: \n";
      // L2::print_vector(n.edges);
      // cout << "\n";
    }
    graph.erase(top.name);
  }

  // stack<Node> generate_stack(vector<Node> graph) {
  //   stack<Node> s;
  //   while (!graph.empty()) {
  //     Node top = graph[0];
  //     graph.erase(graph.begin());
  //     s.push(top);
  //     remove_node(top.name, graph);
  //   }
  //   return s;
  // }


  stack<Node> generate_stack(map<string, Node> graph) {
    stack<Node> s;
    // if (graph.size() > 15){
    //   vector<Node> tops = {};
    //   int64_t curr_max = -1;
    //   for(map<string, Node>::iterator it = graph.begin(); it != graph.end(); it++) {
    //     if (find(begin(registers), end(registers), it->first) != end(registers)){
    //       continue;
    //     }
    //     // Node n = it->second;
    //     // cout << it->second.name << " has " << to_string(it->second.edges.size()) << " edges\n";
    //     // L2::print_vector(it->second.edges);
    //     // cout << "\n";
    //     if((it->second.edges.size() > curr_max) && (it->second.edges.size() < registers.size())){
    //       tops = {it->second};
    //       curr_max = it->second.edges.size();
    //     } else if (it->second.edges.size() == curr_max){
    //       tops.push_back(it->second);
    //     }
    //   }
    //
    //   for (Node top : tops){
    //     cout << "removing " << top.name << " with num of edges " << to_string(top.edges.size()) << "\n";
    //     s.push(top);
    //     remove_node(top, graph);
    //   }
    // }
    while (graph.size() > 15){
      vector<Node> tops = {};
      int curr_max = 0;
      for(map<string, Node>::iterator it = graph.begin(); it != graph.end(); it++) {
        // cout << "name: " << it->first << "\nedges: ";
        // L2::print_vector(it->second.edges);
        // cout << "\n";
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
        // cout << "removing " << top.name << " with num of edges " << to_string(top.edges.size()) << "\n";
        s.push(top);
        remove_node(top, graph);
      }

    }
    return s;

    // while (!graph.empty()) {
    //   Node top = graph[0];
    //   graph.erase(graph.begin());
    //   s.push(top);
    //   remove_node(top.name, graph);
    // }
    // return s;
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
    // cout << "in add node\n";
    // cout << "node name: "  << node.name << "\nedges: ";
    // L2::print_vector(edges);
    // cout << "\n";
    // cout << "\n\nold graph: ";
    // for(map<string, Node>::iterator it = new_g.begin(); it != new_g.end(); it++) {
    //
    //   Node n = it->second;
    //   cout << "Name: " << n.name << "\n";
    //   cout << "Edges: ";
    //   L2::print_vector(n.edges);
    //   cout << "\n";
    // }


    new_g.insert(pair<string,Node>(node.name, node));
    for(map<string, Node>::iterator it = new_g.begin(); it != new_g.end(); it++) {
      // Node n = it->second;
      if (find(begin(edges), end(edges), it->second.name) != end(edges)){
        if (find(begin(new_g[it->second.name].edges), end(new_g[it->second.name].edges), node.name) == end(new_g[it->second.name].edges)){
          new_g[it->second.name].edges.push_back(node.name);
        }
        new_g[node.name].edges.push_back(it->second.name);
      }
      // it->second = n;
    }


  }

  void assign_color(string name, map<string, Node> &new_g, vector<string> &to_spill) {
    // cout << "name: " << name << "\n";
    // cout << "old color: " << new_g[name].color << "\n";
    vector<string> conflicts = get_color_conflicts(name, new_g);
    // cout << "conflicts size: " << conflicts.size() << "\n";
    // cout << "conflicts: ";
    // L2::print_vector(conflicts);
    // cout << endl;

    for (string color: registers) {
      if (find(begin(conflicts), end(conflicts), color) == end(conflicts)) {
        // cout << "Assigning color "<< color << endl;
        new_g[name].color = color;
        // for(map<string, Node>::iterator it = graph.begin(); it != graph.end(); it++) {
        //   Node n = it->second;
        //   cout << n.name << endl;
        //   cout << "Color: " << n.color << endl;
        // }
        // add_edges(graph, new_n.name, n.edges);
        return;
      }
    }
    // cout << "didn't find available color\n";
    to_spill.push_back(name);
  }

  bool assign_colors(map<string, Node> &g, vector<string> &to_spill) {
    // cout << "generating graph vector...\n";
    // vector<L2::Node> graph_vector = generate_graph_vector(g);
    //
    // // cout << "sorting...\n";
    // sort_graph(graph_vector);
    // for (L2::Node nn : graph_vector){
    //   cout << "Name: " << nn.name << "\n";
    //   vector<string> conflicts = get_color_conflicts(nn, g);
    //   cout << "conflicts size: " << nn.edges.size() << "\n";
    // }

    // cout << "generating stack...\n";
    // stack<Node> stack = generate_stack(graph_vector);
    stack<Node> stack = generate_stack(g);

    // cout << "coloring regs...\n";
    // cout << "In graph" << endl;

    map<string, Node> new_g = color_registers();
    while (!stack.empty()) {
      Node n = stack.top();
      stack.pop();

      Node new_n;
      new_n.name = n.name;
      add_node(new_g, new_n, g[new_n.name].edges);

      // // add node
      // vector<string> edges = g[new_n.name].edges;
      // new_g.insert(pair<string,Node>(new_n.name, new_n));
      // for(map<string, Node>::iterator it = new_g.begin(); it != new_g.end(); it++) {
      //   Node n = it->second;
      //   if (find(begin(edges), end(edges), n.name) != end(edges)){
      //     new_g[n.name].edges.push_back(new_n.name);
      //     new_g[new_n.name].edges.push_back(n.name);
      //   }
      // }

      // cout << "in assign_colors, name: " << n.name << "...\n";
      // cout << "\n\ncurr graph: ";
      // for(map<string, Node>::iterator it = new_g.begin(); it != new_g.end(); it++) {
      //
      //   Node n = it->second;
      //   cout << "Name: " << n.name << "\n";
      //   cout << "Edges: ";
      //   L2::print_vector(n.edges);
      //   cout << "\n";
      // }
      assign_color(new_n.name, new_g, to_spill);

      // //assign color
      // // cout << "name: " << new_n.name << "\n";
      // // cout << "old color: " << new_n.color << "\n";
      // vector<string> conflicts = get_color_conflicts(new_n, new_g);
      // // cout << "conflicts size: " << conflicts.size() << "\n";
      // // cout << "conflicts: ";
      // // L2::print_vector(conflicts);
      // // cout << endl;
      //
      // for (string color: registers) {
      //   if (find(begin(conflicts), end(conflicts), color) == end(conflicts)) {
      //     cout << "Assigning color "<< color << endl;
      //     new_n.color = color;
      //     // if (graph.count(n.name)) {
      //     //   cout << "already in graph" << endl;
      //     //   auto ex = graph[n.name];
      //     //   cout << "WTF " << ex.name << " color " << ex.color << endl;
      //     // }
      //     // cout << "before insertion size: " << graph.size() << endl;
      //     //
      //     // cout << "after insertion size: " << graph.size() << endl;
      //     // for(map<string, Node>::iterator it = graph.begin(); it != graph.end(); it++) {
      //     //   Node n = it->second;
      //     //   cout << n.name << endl;
      //     //   cout << "Color: " << n.color << endl;
      //     // }
      //     // add_edges(graph, new_n.name, n.edges);
      //     break;
      //   }
      // }
      // // cout << "didn't find available color\n";
      // to_spill.push_back(new_n.name);
      //cout << "assigned node "<< n.name << " color " << colored_graph[n.name].color << "...\n";
    }
    g = new_g;

    return !to_spill.empty();
  }
}
