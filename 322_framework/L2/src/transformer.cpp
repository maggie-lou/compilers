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
  void add_edges(vector<vector<string>> &graph, vector<string> nodes, map<string, int> &name_index){
    for (int i = 0; i < nodes.size(); i++){
      int index = name_index[nodes[i]];
      graph[index].insert(graph[index].end(), nodes.begin(), nodes.end());
    }
  }

  void get_graph(Program p, vector<vector<string>> &graph, map<string, int> &name_index){
    auto instructions = (p.functions.front())->instructions;
    vector<vector<string>> kill(instructions.size());
    vector<vector<string>> in(instructions.size());
    vector<vector<string>> out(instructions.size());
    L2::get_in_out_sets(p, in, out, kill);

    vector<string> registers = {"r10", "r11", "r12", "r13", "r14", "r15",
                                "r8", "r9", "rax", "rbp", "rbx", "rcx", "rdi",
                                "rdx", "rsi"};
    int num_registers = registers.size();
    int next_index = num_registers;
    for (vector<string> instruction_kill : kill){
      for (string s : instruction_kill){
        if (!name_index.count(s)){
          name_index.insert(pair<string,int>(s,next_index));
          next_index++;
          graph.push_back({});
        }
      }
    }

    // Connect a register to all other registers
    add_edges(graph, registers, name_index);

    // Connect each pair of variables that belong to the same IN or OUT set
    for (vector<string> in_set : in){
      add_edges(graph, in_set, name_index);
    }
    for (vector<string> out_set : out){
      add_edges(graph, out_set, name_index);
    }

    for (int i = 0; i < instructions.size(); i++){
      // Add L1 Constraints
      Assignment* assignment_instruction = dynamic_cast<Assignment*>(instructions.at(i));
      if (assignment_instruction) {
        // var in rhs of sop op can only be assigned to rcx
        if (assignment_instruction->op == "<<=" || assignment_instruction->op == ">>=") {
          if (Var_item* var = dynamic_cast<Var_item*>(assignment_instruction->s)){
            string source = var->var_name;
            int source_graph_index = name_index[source];
            graph[source_graph_index].insert(graph[source_graph_index].end(), registers.begin(), registers.end());
            graph[source_graph_index].erase(remove(graph[source_graph_index].begin(), graph[source_graph_index].end(), "rcx"), graph[source_graph_index].end());
            int rcx_index = name_index["rcx"];
            for (int reg = 0; reg < num_registers; reg++) {
              if (reg == rcx_index) continue;
              graph[reg].push_back(source);
            }
          }
        } else if (assignment_instruction->op == "<-") {
          // Don't add edges to variables and registers during assignment op
          bool s_flag = false;
          if (Var_item* var = dynamic_cast<Var_item*>(assignment_instruction->s)){
            s_flag = true;
          }
          if (Register_item* reg = dynamic_cast<Register_item*>(assignment_instruction->s)){
            s_flag = true;
          }
          if (s_flag){
            bool d_flag = false;
            if (Var_item* var = dynamic_cast<Var_item*>(assignment_instruction->d)){
              d_flag = true;
            }
            if (Register_item* reg = dynamic_cast<Register_item*>(assignment_instruction->d)){
              d_flag = true;
            }
            if (d_flag){
              continue;
            }
          }
        }
      }

      // Connect variables in KILL[i] with those in OUT[i]
      for (int j = 0;  j < kill[i].size(); j++){
        int index = name_index[kill[i][j]];
        graph[index].insert(graph[index].end(), out[i].begin(), out[i].end());
      }
      for (int j = 0;  j < out[i].size(); j++){
        int index = name_index[out[i][j]];
        graph[index].insert(graph[index].end(), kill[i].begin(), kill[i].end());
      }
    }
  }

  void generate_graph(Program p){
    vector<vector<string>> graph(15);
    map<string, int> name_index = {
      {"r10", 0}, {"r11", 1}, {"r12", 2}, {"r13", 3},
      {"r14", 4}, {"r15", 5}, {"r8", 6}, {"r9", 7},
      {"rax", 8}, {"rbp", 9}, {"rbx", 10}, {"rcx", 11},
      {"rdi", 12}, {"rdx", 13}, {"rsi", 14}
    };
    get_graph(p, graph, name_index);
    for(map<string,int>::iterator iter = name_index.begin(); iter != name_index.end(); ++iter) {
      string key = iter-> first;
      int graph_index = iter->second;

      // Remove duplicates
      vector<string> node_edges = graph[graph_index];
      sort(node_edges.begin(), node_edges.end());
      node_edges.erase( unique (node_edges.begin(), node_edges.end() ), node_edges.end());

      // Remove itself from graph
      node_edges.erase(remove(node_edges.begin(), node_edges.end(), key), node_edges.end());
      graph[graph_index] = node_edges;
      if (key[0] != '%'){
        cout << key << " ";
      } else {
        cout << key.substr(1) << " ";
      }
      L2::print_vector(graph[graph_index]);
      cout << "\n";
    }
    return;
  }
}
