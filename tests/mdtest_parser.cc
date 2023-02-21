#include "json.hpp"

#include "Parser.h"
#include "Tokenizer.h"
#include "Exception.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>

static bool operator==(const json::value& lhs, const json::value& rhs);

static void test_parser() {
  const char* filename = __ROOT__ "tests/tests.json";
  std::ifstream ifs(filename);
  if (!ifs.is_open())
  {
    std::cout << filename << " open failed" << std::endl;
  } 

  std::stringstream ss;
  ss << ifs.rdbuf();
  
  auto parse_opt = json::parse(ss.str());
  if (!parse_opt) {
    std::cout << "parse error: " << filename << std::endl;
    return;
  }
  
  auto tests = parse_opt.value();
  auto tests_list = tests["tests_list"].as_array();
 
  auto it = tests_list.begin();

  int success = 0;
  int fail = 0;
  std::for_each(
      tests_list.begin(), tests_list.end(), [&](const json::value &sample) {
        try {
          letter::Parser parser;
          std::string program;
          if (sample.find("program")) {
            program = sample.at("program").as_string();
          } else if (sample.find("program_file")) {
            std::ifstream p_ifs(sample.at("program_file").as_string());
            if (!p_ifs.is_open()) {
              std::cout << sample.at("program_file").as_string()
                        << " open failed" << std::endl;
              ++fail;
              return;
            }

            std::stringstream ss;
            ss << p_ifs.rdbuf();
            program = ss.str();
          }

          auto res = parser.parse(program);

          auto &&expected_result = sample.at("result");
          if (!(res == expected_result)) {
            ++fail;
            std::cout << ">> test failure: " << std::endl;
            std::cout << "program: \"" << program << "\"" << std::endl;
            std::cout << "expected res:\n"
                      << expected_result.format(true) << std::endl;
            std::cout << "actual res:\n" << res.format(true) << std::endl;
          } else {
            ++success;
          }
        } catch (std::exception &e) {
          ++fail;
          std::cout << e.what() << std::endl;
        }
      });

  std::cout << "test parser completed\n"
    << "> Result:\nsuccess: " << success << "\nfail: " << fail << std::endl;

}

int main(int argc, char** argv) {
  
  test_parser();
  return 0;
}


bool operator==(const json::value& lhs, const json::value& rhs)
{
  if (lhs.type() != rhs.type()) return false;
  
  // same type
  // number
  if (lhs.is_number()) {
    return lhs.as_double() == rhs.as_double() && lhs.as_integer() == rhs.as_integer();
  }

  // string
  if (lhs.is_string()) {
    return lhs.as_string() == rhs.as_string();
  }

  // bool
  if (lhs.is_boolean()) {
    return lhs.as_boolean() == rhs.as_boolean();
  }
  
  // null
  if (lhs.is_null()) {
    return rhs.is_null();
  }
  
  // array
  if (lhs.is_array()) {
    auto& la = lhs.as_array();
    auto& ra = rhs.as_array();
    
    if (la.size() != ra.size()) return false;
    for (size_t i = 0; i < la.size(); ++i) {
      if (!(la[i] == ra[i])) return false;
    }
  }

  // object
  if (lhs.is_object()) {
    auto& lo = lhs.as_object();
    auto& ro = rhs.as_object();

    for (auto&& li : lo) {
      if (!ro.find(li.first)) return false;

      if (!(li.second == ro.at(li.first))) return false;
    }
  } 

  return true;
}



