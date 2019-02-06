#include <utils.h>
#include <vector>
#include <iostream>
#include <string>
#include <iterator>
#include <L2.h>

using namespace std;

namespace L2{

  void print_vector(vector<string> v) {
    for (auto item: v) {
      if (item[0] != '%'){
        cout << item << " ";
      } else {
        cout << item.substr(1) << " ";
      }
    }
  }

  void print_function(Function* f){
    cout << '(' << f->name << "\n\t" << to_string(f->arguments) << " " << to_string(f->locals) << endl;

    auto instructions = f->instructions;
    for (auto i : instructions){
      if (Assignment* assignment = dynamic_cast<Assignment*>(i)){
        cout << "\t" << assignment->d->item_to_string() << " " << assignment->op << " " << assignment->s->item_to_string() << "\n";
      }
    }
    cout << ')';
  }
}
