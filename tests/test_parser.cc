#include <iostream>
#include "Parser.h"
#include "ElapsedTimer.h"
#include <string>


int main(int argc, char **argv) {
  
  letter::Parser parser;

  std::string program = R"(
    
    y +=x * 2;

  )";
  json::value ret;
  
  letter::ElapsedTimer t("test_parser parse");

  ret = parser.parse(program);
    
  std::cout << ret.format() << std::endl;
  
  return 0;
}
