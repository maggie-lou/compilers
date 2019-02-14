// TODO have a list/set of labels & variables inside Function

// define all classes
// design tiles
// add operands
// tree generation
// tile matching

#include <L3.h>
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
#include <utils.h>

int main(
  int argc,
  char **argv
){
  auto p = L3::parse_file(argv[optind]);
  return 0;
}
