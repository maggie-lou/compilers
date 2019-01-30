#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>
#include <iterator>

#include <analysis.h>

using namespace std;

namespace L2{
  vector<string> get_union(vector<string> v1, vector<string> v2)  {
    set<string> set1(v1.begin(), v1.end());
    set<string> set2(v2.begin(), v2.end());
    set<string> union_set;
    // set_union(set1.begin(), set1.end(), set2.begin(), set2.end(),
    //           back_inserter(union_set));
    // vector<string> union_vec(union_set.begin(), union_set.end());
    return union_vec;
  }

  vector<string> get_difference(vector<string> v1, vector<string> v2)  {
    set<string> set1(v1.begin(), v1.end());
    set<string> set2(v2.begin(), v2.end());
    set<string> diff_set;
    // set_difference(set1.begin(), set1.end(), set2.begin(), set2.end(),
    //               inserter(diff_set, diff_set.begin()));
    // vector<string> diff_vec(diff_set.begin(), diff_set.end());
    return diff_vec;
  }

  void generate_in_out_sets(Program p){

    // Print to standard out, not a file
    std::cout << ".text\n"
               << "\tretq\n";

    /*
     * Generate target code
     */
    auto instructions = (p.functions.front())->instructions;
    vector<vector<string>> gen(instructions.size());
    vector<vector<string>> kill(instructions.size());
    vector<vector<string>> in(instructions.size());
    vector<vector<string>> out(instructions.size());
    bool changes = false;

    for (int j=0; j<instructions.size(); j++) {
      auto current_i = instructions[j];
      gen[j] = current_i->generate_gen();
      kill[j] = current_i->generate_kill();
      in[j] = {};
      out[j] = {};
    }
    for (int j=0; j<instructions.size(); j++) {
      cout << to_string(j) << endl;
      copy(gen[j].begin(), gen[j].end(), ostream_iterator<char>(cout, " "));
      cout << "\n";
      copy(kill[j].begin(), kill[j].end(), ostream_iterator<char>(cout, " "));
      cout << "\n";
    }


    do {
      for (int j=instructions.size()-1; j>=0; j--) {
        vector<string> diff = get_difference(out[j], kill[j]);
        in[j] = get_union(gen[j], diff);
        // out[j] =
      }
    } while (changes);

    // for (auto f : p.functions){
    //   outputFile << "_" << f->name.substr(1) << ":" << endl;
    //   int local_number = f->locals;
    //   int arg_number = f->arguments;
    //   if (local_number != 0){
    //     int down = 8  * local_number;
    //     outputFile << "\tsubq $" << std::to_string(down) << ", %rsp\n";
    //   }
    //   for (auto instruction : f->instructions){
    //     outputFile << instruction->compile();
    //   }
    // }

    /*
     * Close the output file.
     */

    return ;
  }
}
