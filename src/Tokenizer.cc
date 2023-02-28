#include "Tokenizer.h"
#include "Exception.h"
#include "ElapsedTimer.h"

#include "json.hpp"

#include <regex>
#include <iostream>
#include <vector>
#include <optional>
#include <map>

namespace letter {

Tokenizer::Tokenizer() 
  : m_cursor(0) {

}

Tokenizer::Tokenizer(const std::string& string) 
  : m_string(string), m_cursor(0) {

}

void Tokenizer::init(const std::string& string) {
  this->m_string = string;
  this->m_cursor = 0;
}

/**
 * @brief: vector of the spec table
 * if type == std::nullopt, means just skip this pattern if matched
 */
static const std::vector<std::pair<std::regex, std::optional<std::string>>> s_spec_vec = {
  {std::regex{R"(^;)"}, ";"},                       // ;
  {std::regex{R"(^\s+)"}, std::nullopt},            // white space
  {std::regex{R"(^\d+)"}, "NUMBER"},                // numbers
  {std::regex{R"(^\"[^\"]*\")"}, "STRING"},            // string with double quote
  {std::regex{R"(^\'[^\']*\')"}, "STRING"},            // string with single quote
  {std::regex{R"(^\/\/.*)"}, std::nullopt},           // comments start with "//"
  {std::regex{R"(^\/\*[\s\S]*?\*\/)"}, std::nullopt},   // documentation comment "/* */"
  {std::regex{R"(^\{)"}, "{"},
  {std::regex{R"(^\})"}, "}"},
  {std::regex{R"(^[+\-])"}, "ADDITIVE_OPERATOR"},
  {std::regex{R"(^[\*\/])"}, "MULTIPLICATIVE_OPERATOR"},
  {std::regex{R"(^\()"}, "("},
  {std::regex{R"(^\))"}, ")"}
};

/**
 * @brief: Try to match(search) regex pattern `regexp` in str
 * @return: matched result if matched, `std::nullopt` if not matched
 */
static std::optional<std::string> 
_match(const std::regex& regexp, const std::string& str) {
  // std::cout << "_match cur str: \"" << str << "\"" << std::endl;
  // ElapsedTimer t("match timer");
  std::smatch m;
  if (std::regex_search(str.begin(), str.end(), m, regexp)) {
    return m[0].str();
  } else {
    return std::nullopt;
  }
}

json::value Tokenizer::getNextToken() {
  if (!this->hasMoreTokens()) {
    // std::cout << "[DEBUG] has no more tokens!" << std::endl;
    return {};
  }
  
  const auto str = this->m_string.substr(this->m_cursor);

  for (auto&& [regexp, token_type_opt] : s_spec_vec) {
    auto&& token_value_opt = _match(regexp, str);
    if (!token_value_opt) {
      // if value is nullopt, means does not match
      continue; // continue trying to match next regex pattern
    }
    
    // matched
    
    // std::cout << "type:" 
    //   << token_type_opt.value_or("null_type") << std::endl;

    // std::cout << "value:["
    //   << token_value_opt.value() << "]" << std::endl;

    // assert whether it is the first few chars
    auto&& value = token_value_opt.value();
    if (value != str.substr(0, value.size())) {
      throw Exception("Unexpected failure. str:" + str + ", value:" + value);
    }

    // increase the cursor, to point to the next possible token start
    this->m_cursor += value.size();

    if (!token_type_opt) {
      // if matched, but token type is null, means to skip this token
      // such as: whitespace, comments etc.
      return this->getNextToken(); // skip this, find next token
    }
    
    return json::object{
      {"type", token_type_opt.value()}, 
      {"value", token_value_opt.value()}
    };
  }
  
  // After trying all the regex match, still not match, then throw
  throw Exception("Unexpected token: \"" + str.substr(0, 1) + "\"");

  /*
  // Numbers:
  std::smatch m;
  if (std::regex_search(str.begin(), str.end(), m, std::regex(R"(^\d+)"))) {
    this->m_cursor += m[0].length();
    // std::cout << m[0].str() << std::endl;
    return json::object{
      {"type", "NUMBER"},
      {"value", m[0].str()}
    };
  }
  // if (std::isdigit(this->m_string[m_cursor])) {
  //   std::string number;
  //   while (std::isdigit(this->m_string[this->m_cursor])) {
  //     number += this->m_string[this->m_cursor++];
  //   }

  //   return json::object{
  //     {"type", "NUMBER"},
  //     {"value", number}
  //   };
  // }

  // String: "..."
  if (std::regex_search(str.begin(), str.end(), m, std::regex(R"("[^"]*")"))) {
    this->m_cursor += m[0].length();
    return json::object{
      {"type", "STRING"},
      {"value", m[0].str()}
    };
  }
  
  // String: '...'
  if (std::regex_search(str.begin(), str.end(), m, std::regex(R"('[^']*')"))) {
    this->m_cursor += m[0].length();
    return json::object{
      {"type", "STRING"},
      {"value", m[0].str()}
    };
  }

  // if (this->m_string[0] == '\"') {
  //   std::string s;
  //   while ( this->m_string[++this->m_cursor] != '\"' && !this->isEOF() ) {
  //     s += this->m_string[this->m_cursor];
  //   }
  //   return json::object{
  //     {"type", "STRING"},
  //     {"value", s}
  //   };
  // }
  */

}

}
