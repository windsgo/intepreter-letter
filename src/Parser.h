#pragma once

#include <string>
#include <memory>

#include "json.hpp"

#include "Tokenizer.h"

namespace letter {

class Parser {
private:
  std::string m_string;
  
  std::unique_ptr<Tokenizer> m_tokenizer;

  json::value m_lookahead;  

public:
  Parser();

  json::value parse(const std::string &str);

  json::value Program();
  
  json::value StatementList();

  json::value Statement();
  json::value ExpressionStatement();
  json::value Expression();

  json::value Literal();
  json::value StringLiteral(); 
  json::value NumericLiteral();

private:
  json::value _eat(const std::string& token_type);
};

} // namespace letter
