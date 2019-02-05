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
}
