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
      } else if (AssignmentCmp* ass_cmp = dynamic_cast<AssignmentCmp*>(i)) {
        cout << "\t" << ass_cmp->d->item_to_string() << " <- " << ass_cmp->c.to_string() << "\n";
      } else if (Cjump* cjump = dynamic_cast<Cjump*>(i)) {
        cout << "\tcjump " << cjump->c.to_string() << " " << cjump->label1 << " " << cjump->label2 << "\n";
      } else if (Cjump_fallthrough* cjump_fallthrough = dynamic_cast<Cjump_fallthrough*>(i)) {
        cout << "\tcjump " << cjump_fallthrough->c.to_string() << " " << cjump_fallthrough->label << "\n";
      }
    }
    cout << ')';
  }
}
