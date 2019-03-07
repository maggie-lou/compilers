#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <iostream>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
#include <iostream>

#include <parser.h>
#include <code_generator.h>
#include <name_binding.h>

using namespace std;

int main(
  int argc,
  char **argv
  ){

  auto p = LB::parse_file(argv[optind]);
  LB::binding_name(p);
  LB::generate_code(p);
  return 0;
}
