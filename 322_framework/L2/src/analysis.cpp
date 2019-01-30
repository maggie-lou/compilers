#include <string>
#include <iostream>
#include <fstream>

#include <analysis.h>

using namespace std;

namespace L2{
  void generate_in_out_sets(Program p){

    // Print to standard out, not a file
    std::cout << ".text\n"
               << "\tretq\n";

    /*
     * Generate target code
     */
    auto i = (p.functions.front())->instructions;
    vector<vector<std::string>> gen(instructions.size());
    vector<vector<std::string>> kill(instructions.size());
    vector<vector<std::string>> in(instructions.size());
    vector<vector<std::string>> out(instructions.size());
    bool changes = false;

    for (int j=0; j<i.size(); j++) {
      auto current_i = i[j];
      gen[j] = generate_gen(current_i);
      kill[j] = generate_kill(current_i);
      in[j] = {};
      out[j] = {};
    }

    do {
      for (int j=i.size()-1; j>=0; j--) {
        in[j] = union(gen[j], difference(out[j], kill[j]));
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

  vector<std::string> generate_gen(Instruction* i) {

  }
  vector<std::string> generate_kill(Instruction* i) {

  }
  vector<std::string> union(vector<std::string> v1, vector<std::string> v2)  {

  }
  vector<std::string> difference(vector<std::string> v1, vector<std::string> v2)  {

  }
}

