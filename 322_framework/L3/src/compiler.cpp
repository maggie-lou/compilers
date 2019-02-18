// TODO have a list/set of labels & variables inside Function

// tree merging
// design tiles
// tile matching


#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
#include <iostream>
#include <unordered_map>
#include <stack>

#include <parser.h>
#include <utils.h>
#include <context_identification.h>
#include <tree_generation.h>
#include <analysis.h>
#include <tile_matching.h>
#include <code_generator.h>
#include <L3.h>

using namespace std;

int main(
  int argc,
  char **argv
){
  auto p = L3::parse_file(argv[optind]);

  ofstream outputFile;
  outputFile.open("prog.L2");
  outputFile << "(:main\n";

  for (auto f : p.functions){
    auto instructions = f->instructions;
    auto label_map = L3::create_label_map(p, f);

    auto contexts = L3::generate_contexts(f);
    cout << "contexts size " << contexts.size() << endl;

    vector<vector<string>> in(instructions.size());
    vector<vector<string>> out(instructions.size());
    cout << "About to generate in out sets"<<endl;
    L3::get_in_out_sets(f, in, out);

    vector<stack<string>> all_l2_instructions;
    for (auto context : contexts){
      cout << "About to generate trees" << endl;
      auto trees = L3::generate_and_merge_trees(context, in, out, label_map);
    cout << "trees size " << trees.size() << endl;
      stack<string> l2_instructions = L3::generate_l2_instructions(trees, p.longest_label_name, p.label_count);
    cout << "l2 instructions size " << l2_instructions.size() << endl;
      all_l2_instructions.push_back(l2_instructions);
    }

    L3::generate_code(f, all_l2_instructions, label_map, outputFile);
  }
  outputFile << ")\n";
  outputFile.close();
  return 0;
}
