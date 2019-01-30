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
    vector<string> union_vec;
    set_union(v1.begin(), v1.end(), v2.begin(), v2.end(),
              back_inserter(union_vec));
    set<string> union_set(union_vec.begin(), union_vec.end());
    vector<string> ans(union_set.begin(), union_set.end());
    return ans;
  }

  vector<string> get_difference(vector<string> v1, vector<string> v2)  {
    vector<string> diff_vec;
    set_difference(v1.begin(), v1.end(), v2.begin(), v2.end(),
              inserter(diff_vec, diff_vec.begin()));
    set<string> diff_set(diff_vec.begin(), diff_vec.end());
    vector<string> ans(diff_set.begin(), diff_set.end());
    return ans;
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
    // cout << "gen\n";
    // for (int i = 0; i < gen.size(); i++)
    //   {
    //       for (int j = 0; j < gen[i].size(); j++)
    //       {
    //           cout << gen[i][j] << " ";
    //       }
    //       cout << "\n";
    //   }
    // cout << "kill\n";
    // for (int i = 0; i < kill.size(); i++)
    //   {
    //       for (int j = 0; j < kill[i].size(); j++)
    //       {
    //           cout << kill[i][j] << " ";
    //       }
    //       cout << "\n";
    //   }

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
