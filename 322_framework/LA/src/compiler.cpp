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
#include <code_generation.h>

using namespace std;

int main(
  int argc,
  char **argv
  ){

  auto p = LA::parse_file(argv[optind]);
  LA::generate_code(p);

  return 0;
}
