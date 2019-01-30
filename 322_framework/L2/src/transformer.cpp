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
      for (int j = 0; j < nodes.size(); j++){
        if (nodes[i] != nodes[j]){
          graph[name_index[nodes[i]]].push_back(nodes[j]);
        }
      }
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

    add_edges(graph, registers, name_index);

    for (vector<string> in_set : in){
      add_edges(graph, in_set, name_index);
    }
    for (vector<string> out_set : out){
      add_edges(graph, out_set, name_index);
    }

    for (int i = 0; i < instructions.size(); i++){
      for (int j = 0;  j < kill[i].size(); j++){
        graph[name_index[kill[i][j]]].insert(graph[name_index[kill[i][j]]].end(),
                                            out[i].begin(), out[i].end());
      }
      for (int j = 0;  j < out[i].size(); j++){
        graph[name_index[out[i][j]]].insert(graph[name_index[out[i][j]]].end(),
                                            kill[i].begin(), kill[i].end());
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
      cout << iter->first << " ";
      L2::print_vector(graph[iter->second]);
      cout << "\n";
    }
    return;
  }
}
