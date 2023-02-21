#include "Parser.h"
#include "Exception.h"
#include "json.hpp"

#include <stdexcept>
#include <system_error>

namespace letter {

Parser::Parser() 
  : m_string(), m_tokenizer(std::make_unique<Tokenizer>()) {

}

json::value Parser::parse(const std::string &str) {
  this->m_string = str;
  this->m_tokenizer->init(str);

  this->m_lookahead = this->m_tokenizer->getNextToken();
  return this->Program();
}

json::value Parser::Program() {
  return json::object{
    {"type", "Program"},
    {"body", this->StatementList()}
  };
}

json::value Parser::StatementList() {
  json::array statement_list;

  statement_list.emplace_back(this->Statement());
  while (!this->m_lookahead.empty()) {
    statement_list.emplace_back(this->Statement());
  } 

  return statement_list;
}


json::value Parser::Statement() {
  return this->ExpressionStatement();
}

/**
 * ExpressionStatement
 * a type of `Statement`
 * consists of an `Expression` and a `;`
 */
json::value Parser::ExpressionStatement() {
  auto expression = this->Expression();
  this->_eat(";");
  
  // json::value o;
  // o["type"] = "ExpressionStatement";
  // o |= expression.as_object();

  return json::object{
    {"type", "ExpressionStatement"},
    {"expression", expression}
  };
}

/**
 * Expression
 */
json::value Parser::Expression() {
  return this->Literal();
}

/**
 * Literal
 * consistent of an `Expression`
 */
json::value Parser::Literal()
{
  auto type = this->m_lookahead["type"].as_string(); 
  if (type == "NUMBER") {
    return this->NumericLiteral();
  } else if (type == "STRING") {
    return this->StringLiteral();
  }

  throw Exception("Unexpected literal production");
}

json::value Parser::StringLiteral() {
  json::value token = this->_eat("STRING");
 
  auto str = token["value"].as_string();
  return json::object{
    {"type", "StringLiteral"},
    {"value", str.substr(1, str.size() - 2)} // 去除前后的引号
  };
}

json::value Parser::NumericLiteral() {
  json::value token = this->_eat("NUMBER");

  return json::object{
    {"type", "NumericLiteral"},
    {"value", std::stoi(token["value"].as_string())}
  };
}

json::value Parser::_eat(const std::string& token_type) {
  json::value token = this->m_lookahead;

  if (token.empty()) {
    throw Exception("Unexpected end of input, expected: " + token_type);
  }

  if (token["type"].as_string() != token_type) {
    throw Exception("Unexpected token: " + token["value"].to_string() + ", expected: " + token_type);
  }

  this->m_lookahead = this->m_tokenizer->getNextToken();

  return token;
}

} // namespace letter
