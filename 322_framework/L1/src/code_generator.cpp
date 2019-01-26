#include <string>
#include <iostream>
#include <fstream>

#include <code_generator.h>

using namespace std;

namespace L1{
  void generate_code(Program p){

    /*
     * Open the output file.
     */
    std::ofstream outputFile;
    outputFile.open("prog.S");

    outputFile << ".text\n"
               << "\t.global go\n"
               << "go:\n"
               << "\tpushq %rbx\n"
               << "\tpushq %rbp\n"
               << "\tpushq %r12\n"
               << "\tpushq %r13\n"
               << "\tpushq %r14\n"
               << "\tpushq %r15\n"
               << "\tcall _" << p.entryPointLabel.substr(1) << endl
               << "\tpopq %r15\n"
               << "\tpopq %r14\n"
               << "\tpopq %r13\n"
               << "\tpopq %r12\n"
               << "\tpopq %rbp\n"
               << "\tpopq %rbx\n"
               << "\tretq\n";

    /*
     * Generate target code
     */
    for (auto f : p.functions){
      outputFile << "_" << f->name.substr(1) << ":" << endl;
      int local_number = f->locals;
      int arg_number = f->arguments;
      if (local_number != 0){
        int down = 8  * local_number;
        outputFile << "\tsubq $" << std::to_string(down) << ", %rsp\n";
      }
      for (auto instruction : f->instructions){
        outputFile << instruction->compile();
      }
    }

    /*
     * Close the output file.
     */
    outputFile.close();

    return ;
  }
}
