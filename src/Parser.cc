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
  while (!this->m_lookahead.empty() && this->m_lookahead["type"] != stop_lookahead_tokentype.value_or("_null")) {
    // stop_lookahead_tokentype 是指结束查找语句的标识，如块语句从'{'查找到下一个'}'为止
    statement_list.emplace_back(this->Statement());
  } 

  return statement_list;
}

/**
 * Statement
 *  : ExpressionStatement
 *  | BlockStatement
 *  | EmptyStatement
 *  ;
 */
json::value Parser::Statement() {
  assert(!this->m_lookahead.empty());

  auto&& type = this->m_lookahead["type"].as_string();
  if (type == "{") {
    return this->BlockStatement();
  } else if (type == ";") {
    return this->EmptyStatement();
  } else {
    return this->ExpressionStatement();
  }
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
  
  // if is an empty block, return an empty json::array, which in json is []
  auto&& body = this->m_lookahead["type"].as_string() == "}" ? 
    json::value{json::array{}} : this->StatementList("}");
  
  // std::cout << "meet a BlockStatement: body.empty():" << body.empty() << std::endl;
  // std::cout << "body.is_null:" << body.is_null() << std::endl;
  // std::cout << "body.is_array" << body.is_array() << std::endl;
  // std::cout << "body.as_array().size()" << body.as_array().size() << std::endl;

  this->_eat("}");

  return json::object{
    {"type", "BlockStatement"},
    {"body", body}
  };
}

/**
 * EmptyStatement
 *  : ";"
 *  ;
 */
json::value Parser::EmptyStatement() {
  this->_eat(";");
  return json::object{
    {"type", "EmptyStatement"}
  };
}

/**
 * Expression
 */
json::value Parser::Expression() {
  return this->AdditiveExpression();
}

/**
 * Generic binary expression.
 */
json::value Parser::_BinaryExpression(std::function<json::value(void)> builder, const std::string& operator_token) {
  auto&& left = builder();

  while (this->m_lookahead["type"].as_string() == operator_token) {
    auto&& op = this->_eat(operator_token);
  
    auto&& right = this->MultiplicativeExpression();

    left = json::object{
      {"type", "BinaryExpression"},
      {"operator", op["value"]},
      {"left", left},
      {"right", right}
    };
  }

  return left;
}

/**
 * AdditiveExpression
 *  : MultiplicativeExpression
 *  | AdditiveExpression ADDITIVE_OPERATOR MultiplicativeExpression
 *  ;
 */
json::value Parser::AdditiveExpression() {
  return _BinaryExpression(std::bind(&Parser::MultiplicativeExpression, this),
      "ADDITIVE_OPERATOR");
}

/**
 * MultiplicativeExpression
 *  : PrimaryExpression
 *  | MultiplicativeExpression ADDITIVE_OPERATOR PrimaryExpression
 *  ;
 */
json::value Parser::MultiplicativeExpression() {
  return _BinaryExpression(std::bind(&Parser::PrimaryExpression, this),
      "MULTIPLICATIVE_OPERATOR");
}

/**
 * PrimaryExpression
 *  : Literal
 *  ;
 */
json::value Parser::PrimaryExpression() {
  auto&& type = this->m_lookahead["type"].as_string();

  if (type == "(") {
    return this->ParenthesizedExpression();
  } else {
    return this->Literal();
  }
}

/**
 * ParenthesizedExpression
 *  : "(" Expression ")"
 *  ;
 */
json::value Parser::ParenthesizedExpression() {
  this->_eat("(");
  auto&& expression = this->Expression(); // here inside ( ) must have AN expression, or throw error

  this->_eat(")");

  return expression;
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
