#include "json.hpp"

#include "ElapsedTimer.h"
#include "Exception.h"
#include "Parser.h"
#include "Tokenizer.h"
#include <algorithm>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>

static bool test_a_program(letter::Parser &parser,
                           const json::value &program_json) {
  try {
    auto &&parse_result = parser.parse(program_json.at("program").as_string());

    auto &&expected_result = program_json.at("result");

    if (parse_result != expected_result) {
      std::cout << ">> test failure: " << std::endl;
      std::cout << "program: \n\"" << program_json.at("program").as_string() << "\""
                << std::endl;
      std::cout << "expected res:\n"
                << expected_result.format(true) << std::endl;
      std::cout << "actual res:\n" << parse_result.format(true) << std::endl;
      return false;
    } else {
      return true;
    }

  } catch (const std::exception &e) {
    std::cout << "exception: " << e.what() << std::endl;
    std::cout << "when testing:\n" << program_json.format() << std::endl;
    return false;
  }
}

static bool test_a_program_file(letter::Parser &parser,
                                const std::string &filename, const json::value& result) {
  std::ifstream ifs(filename);
  if (!ifs.is_open()) {
    std::cout << filename << " open failed" << std::endl;
    return false;
  }

  std::stringstream ss;
  ss << ifs.rdbuf();

  auto&& t = json::object{
    {"program", ss.str()},
    {"result", result}
  };
  
  return test_a_program(parser, t);
}

static bool test_a_json(letter::Parser &parser, const json::value& json_block);

static bool test_a_sub_jsonfile(letter::Parser &parser, const std::string& filename) {
  std::ifstream ifs(filename);
  if (!ifs.is_open()) {
    std::cout << filename << " open failed" << std::endl;
    return false;
  }

  std::stringstream ss;
  ss << ifs.rdbuf();

  auto&& res = json::parse(ss.str());
  
  if (!res) {
    std::cout << "json file: " << filename << " invalid:" << std::endl;
    std::cout << ss.str() << std::endl;
    return false;
  }

  return test_a_json(parser, res.value());
}

static bool test_a_json(letter::Parser &parser, const json::value& json_block) {
  if (json_block.find("program")) {
    return test_a_program(parser, json_block);
  } else if (json_block.find("program_file")) {
    return test_a_program_file(parser, json_block.at("program_file").as_string(), json_block.at("result"));
  } else if (json_block.find("sub_json")) {
    return test_a_sub_jsonfile(parser, json_block.at("sub_json").as_string());
  } else {
    std::cout << "invalid json_block:" << json_block << std::endl;
    return false;
  }
}

static void test_parser() {
  const char* filename = __ROOT__ "tests/tests.json";
  std::ifstream ifs(filename);
  if (!ifs.is_open())
  {
    std::cout << filename << " open failed" << std::endl;
  } 

  std::stringstream ss;
  ss << ifs.rdbuf();
  
  auto&& parse_opt = json::parse(ss.str());
  if (!parse_opt) {
    std::cout << "parse error: " << filename << std::endl;
    return;
  }
  
  auto&& tests = parse_opt.value();
  auto&& tests_list = tests["tests_list"].as_array();
 
  auto it = tests_list.begin();

  int success = 0;
  int fail = 0;

  letter::Parser parser;
  std::for_each(
      tests_list.begin(), tests_list.end(), [&](const json::value &item) {
        if (test_a_json(parser, item)) {
          ++ success;
        } else {
          ++ fail;
        }
      });

  std::cout << "test parser completed\n"
    << "> Result:\nsuccess: " << success << "\nfail: " << fail << std::endl;

}

int main(int argc, char** argv) {
  {
    letter::ElapsedTimer t("md_test_parser total time");
    test_parser();
  }
  return 0;
}

