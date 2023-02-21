#include <iostream>
#include "Parser.h"
#include <string>

int main(int argc, char **argv) {

  letter::Parser parser;

  if (argc != 2) {
    return 1;
  }

  std::string program;

  switch (std::stoi(argv[1])) {
  case 1:
    program = R"("ab")";
    break;
  case 2:
    program = R"('xyz')";
    break;
  case 3:
    program = R"(0123)";
    break;
  case 4:
    program = R"( 1)";
    break;
  case 5:
    program = R"(  "xx" )";
    break;
  case 6:
    program = R"(   
       
    // comments
    // aa
    123

    // xyz
  
    )";
    break;
  case 7:
    program = R"(
    
      /**
       * Hi Doc Comment
       */
      "abc str"
    )";
    break;
  default:
    return 1;
  }

  auto ret = parser.parse(program);

  std::cout << ret.format() << std::endl;

  return 0;
}
