#include "Parser.h"
#include "Exception.h"
#include "json.hpp"

#include <optional>
#include <stdexcept>
#include <system_error>
#include <cassert>
#include <iostream>

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

/**
 * StatementList
 *  : Statement
 *  | StatementList Statement -> Statement Statement Statement Statement
 */
json::value Parser::StatementList(const std::optional<std::string>& stop_lookahead_tokentype/*= std::nullopt*/) {
  json::array statement_list;

  statement_list.emplace_back(this->Statement());
  while (!this->m_lookahead.empty() && this->m_lookahead["type"] != stop_lookahead_tokentype.value_or("null")) {
    statement_list.emplace_back(this->Statement());
  } 

  return statement_list;
}

/**
 * Statement
 *  : ExpressionStatement
 *  | BlockStatement
 *  ;
 */
json::value Parser::Statement() {
  assert(!this->m_lookahead.empty());

  auto&& type = this->m_lookahead["type"].as_string();
  if (type == "{") {
    return this->BlockStatement();
  } else {
    return this->ExpressionStatement();
  }

  return this->ExpressionStatement();
}

/**
 * ExpressionStatement
 *  : Expression ";"
 *  ;
 */
json::value Parser::ExpressionStatement() {
  auto&& expression = this->Expression();
  this->_eat(";");

  return json::object{
    {"type", "ExpressionStatement"},
    {"expression", expression}
  };
}

/**
 * BlockStatement
 *  : "{" OptStatementList "}" (Opt means optional)
 *  ;
 */
json::value Parser::BlockStatement() {
  this->_eat("{");
  
  auto&& body = this->m_lookahead["type"].as_string() == "}" ? json::value{} : this->StatementList("}");

  this->_eat("}");

  return json::object{
    {"type", "BlockStatement"},
    {"body", body}
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
  assert(!this->m_lookahead.empty());
  auto&& type = this->m_lookahead["type"].as_string(); 
  if (type == "NUMBER") {
    return this->NumericLiteral();
  } else if (type == "STRING") {
    return this->StringLiteral();
  }

  throw Exception("Unexpected literal production");
}

json::value Parser::StringLiteral() {
  auto&& token = this->_eat("STRING");
 
  auto&& str = token["value"].as_string();
  return json::object{
    {"type", "StringLiteral"},
    {"value", str.substr(1, str.size() - 2)} // 去除前后的引号
  };
}

json::value Parser::NumericLiteral() {
  auto&& token = this->_eat("NUMBER");

  return json::object{
    {"type", "NumericLiteral"},
    {"value", std::stoi(token["value"].as_string())}
  };
}

json::value Parser::_eat(const std::string& token_type) {
  auto token = this->m_lookahead;

  if (token.empty()) {
    throw Exception("Unexpected end of input, expected: " + token_type);
  }

  if (token["type"].as_string() != token_type) {
    throw Exception("Unexpected token: " + token["value"].to_string() + ", expected: " + token_type);
  }
  
  // TimeCounter t;
  // get next token after eat for lookahead
  this->m_lookahead = this->m_tokenizer->getNextToken();
  
  // return the eaten token
  return token;
}

} // namespace letter
