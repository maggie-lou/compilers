#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <algorithm>
#include <iterator>

#include <transformer.h>
#include <analysis.h>
#include <utils.h>

using namespace std;

namespace L2{
  void add_edges(map<string, Node> &graph, vector<string> nodes){
    for (string node: nodes) {
      graph[node].edges.insert(graph[node].edges.end(), nodes.begin(), nodes.end());
    }
  }

  void generate_graph_variables(Function* f, map<string, Node> &graph){
    auto instructions = f->instructions;
    vector<vector<string>> kill(instructions.size());
    vector<vector<string>> in(instructions.size());
    vector<vector<string>> out(instructions.size());
    L2::get_in_out_sets(f, in, out, kill);

    // Add variables to graph
    for (vector<string> instruction_kill : kill){
      for (string s : instruction_kill){
        if (!graph.count(s)){
          Node n;
          n.name = s;
          n.edges = {};
          graph.insert(pair<string,Node>(s, n));
        }
      }
    }

    // Connect each pair of variables that belong to the same IN or OUT set
    for (vector<string> in_set : in){
      add_edges(graph, in_set);
    }
    for (vector<string> out_set : out){
      add_edges(graph, out_set);
    }

    for (int i = 0; i < instructions.size(); i++){
      // Add L1 Constraints
      Assignment* assignment_instruction = dynamic_cast<Assignment*>(instructions.at(i));
      if (assignment_instruction) {
        // var in rhs of sop op can only be assigned to rcx
        if (assignment_instruction->op == "<<=" || assignment_instruction->op == ">>=") {
          if (Var_item* var = dynamic_cast<Var_item*>(assignment_instruction->s)){
            string source = var->var_name;
            graph[source].edges.insert(graph[source].edges.end(),
                                             L2::registers.begin(), L2::registers.end());
            graph[source].edges.erase(remove(graph[source].edges.begin(),
                                                   graph[source].edges.end(),
                                                   "rcx"),
                                            graph[source].edges.end());
            for(map<string,Node>::iterator iter = graph.begin(); iter != graph.end(); ++iter) {
              string reg = iter-> first;
              if (reg == "rcx") continue;
              graph[reg].edges.push_back(source);
            }
          }
        }
      }

      // Connect variables in KILL[i] with those in OUT[i]
      for (int j = 0;  j < kill[i].size(); j++){
        string kill_var = kill[i][j];
        graph[kill_var].edges.insert(graph[kill_var].edges.end(), out[i].begin(), out[i].end());
      }
      for (int j = 0;  j < out[i].size(); j++){
        string out_var = out[i][j];
        graph[out_var].edges.insert(graph[out_var].edges.end(), kill[i].begin(), kill[i].end());
      }
    }
  }

  void generate_graph_registers(map<string, Node> &graph) {
    for (string r: L2::registers) {
      Node n;
      n.name = r;
      n.edges = {};
      graph.insert(pair<string,Node>(r, n));
    }
    add_edges(graph, L2::registers);
  }

  void print_graph(map<string, Node> graph) {
    for(map<string,Node>::iterator iter = graph.begin(); iter != graph.end(); ++iter) {
      string reg = iter-> first;

      if (reg[0] != '%'){
        cout << reg << " ";
      } else {
        cout << reg.substr(1) << " ";
      }

      L2::print_vector(graph[reg].edges);
      cout << "\n";
    }
  }

  map<string, Node> generate_graph(Function* f){
    map<string, Node> graph;
    generate_graph_registers(graph);
    generate_graph_variables(f, graph);

    for(map<string,Node>::iterator iter = graph.begin(); iter != graph.end(); ++iter) {
      string reg = iter-> first;

      // Remove duplicates
      vector<string> node_edges = graph[reg].edges;
      sort(node_edges.begin(), node_edges.end());
      node_edges.erase( unique (node_edges.begin(), node_edges.end() ), node_edges.end());

      // Remove itself from graph
      node_edges.erase(remove(node_edges.begin(), node_edges.end(), reg), node_edges.end());
      graph[reg].edges = node_edges;
    }
    return graph;
  }

  void generate_and_print_graph(Program p) {
    Function* f = p.functions.front();
    map<string, Node> graph = generate_graph(f);
    print_graph(graph);
  }

}
