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
      for (string to_insert: nodes){
        if ((to_insert != node) && (find(begin(graph[node].edges), end(graph[node].edges), to_insert) == end(graph[node].edges))){
          graph[node].edges.push_back(to_insert);
        }
      }
    }
  }

  void generate_graph_variables(Function* f, map<string, Node> &graph){
    auto instructions = f->instructions;
    vector<vector<string>> kill(instructions.size());
    vector<vector<string>> in(instructions.size());
    vector<vector<string>> out(instructions.size());
    L2::get_in_out_sets(f, in, out, kill);
    // for (int i = 0; i < instructions.size(); i++){
    //   cout << "kill: \n";
    //   L2::print_vector(kill[i]);
    //   cout << "\n";
    //
    //   cout << "in: \n";
    //   L2::print_vector(in[i]);
    //   cout << "\n";
    //
    //   cout << "out: \n";
    //   L2::print_vector(out[i]);
    //   cout << "\n";
    // }

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
    for (vector<string> instruction_in : in){
      for (string s : instruction_in){
        if (!graph.count(s)){
          Node n;
          n.name = s;
          n.edges = {};
          graph.insert(pair<string,Node>(s, n));
        }
      }
    }
    for (vector<string> instruction_out : out){
      for (string s : instruction_out){
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
      // cout << "instruction " << to_string(i) << "\n";
      // cout << "in the beginning\n";
      // L2::print_vector(graph["%v1"].edges);
      // cout << "\n\n";
      // Add L1 Constraints
      Assignment* assignment_instruction = dynamic_cast<Assignment*>(instructions.at(i));
      if (assignment_instruction) {
        // var in rhs of sop op can only be assigned to rcx
        if (assignment_instruction->op == "<<=" || assignment_instruction->op == ">>=") {
          if (Var_item* var = dynamic_cast<Var_item*>(assignment_instruction->s)){
            string source = var->var_name;
            for (string reg: L2::registers){
              if (reg == "rcx") continue;
              if (find(begin(graph[source].edges), end(graph[source].edges), reg) == end(graph[source].edges)){
                graph[source].edges.push_back(reg);
              }
              if (find(begin(graph[reg].edges), end(graph[reg].edges), source) == end(graph[reg].edges)){
                graph[reg].edges.push_back(source);
              }
            }
          }
        }
      }
      // cout << "in the middle\n";
      // L2::print_vector(graph["%v1"].edges);
      // cout << "\n\n";

      // Connect variables in KILL[i] with those in OUT[i]
      for (int j = 0;  j < kill[i].size(); j++){
        string kill_var = kill[i][j];
        for (string to_insert : out[i]){
          if ((to_insert != kill_var) && (find(begin(graph[kill_var].edges), end(graph[kill_var].edges), to_insert) == end(graph[kill_var].edges))){
            graph[kill_var].edges.push_back(to_insert);
          }
        }
      }
      for (int j = 0;  j < out[i].size(); j++){
        string out_var = out[i][j];
        for (string to_insert : kill[i]){
          if ((to_insert != out_var) && (find(begin(graph[out_var].edges), end(graph[out_var].edges), to_insert) == end(graph[out_var].edges))){
            graph[out_var].edges.push_back(to_insert);
          }
        }
      }
      // cout << "at the end\n";
      // L2::print_vector(graph["%v1"].edges);
      // cout << "\n\n";
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

    // L2::print_function(f);

    generate_graph_registers(graph);
    generate_graph_variables(f, graph);

    // for(map<string,Node>::iterator iter = graph.begin(); iter != graph.end(); ++iter) {
    //   cout << iter->first << "\n";
    //   L2::print_vector(iter->second.edges);
    //   cout << "\n\n";
    // }

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
