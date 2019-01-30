#include <utils.h>
#include <vector>
#include <iostream>

using namespace std;

namespace L2{

  void print_vector(vector<string> v) {
    for (auto item: v) {
      cout << item << " ";
    }
  }
}
