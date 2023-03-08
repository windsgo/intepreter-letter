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

  return statement_list; // a json::array, no "type" property
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
 *  : Literal
 *  ;
 */
json::value Parser::Expression() {
  return this->AssignmentExpression();
  // return this->AdditiveExpression();
}

/**
 * AssignmentExpression
 *  : AdditiveExpression
 *  | LeftHandSideExpression AssignmentOperator AssignmentExpression
 *  ;
 */
json::value Parser::AssignmentExpression() {
  // 赋值表达式的运算优先级比BinaryExpression的优先级更低，所以放在更外层
  auto&& left = this->AdditiveExpression();

  if (!this->_isAssignmentOperator(this->m_lookahead)) {
    return left; // if there is no assign op after first 'AdditiveExpression', that is 'AdditiveExpression' itself
  }

  return json::object{
    {"type", "AssignmentExpression"},
    {"operator", this->AssignmentOperator()["value"]},
    {"left", _checkValidAssignmentTarget(left)},
    {"right", this->AssignmentExpression()}
  };
}

/**
 * LeftHandSideExpression
 * : Identifier
 * ;
 */
json::value Parser::LeftHandSideExpression() {
  return this->Identifier();
}

/**
 * Identifier
 * : IDENTIRIFER
 * ;
 */
json::value Parser::Identifier() {
  auto&& name = this->_eat("IDENTIRIFER");
  return json::object{
    {"type", "Identifier"},
    {"name", name["value"]}
  };
}

/**
 * Generic binary expression.
 */
json::value Parser::_BinaryExpression(std::function<json::value(void)> builder, 
    const Tokenizer::TokenType& operator_token) {
  auto&& left = builder();

  while (this->m_lookahead["type"].as_string() == operator_token.value()) {
    auto&& op = this->_eat(operator_token.value());
  
    // auto&& right = this->MultiplicativeExpression();
    auto&& right = builder();

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
  return _BinaryExpression(
      std::bind(&Parser::MultiplicativeExpression, this),
      "ADDITIVE_OPERATOR");
}

/**
 * MultiplicativeExpression
 *  : PrimaryExpression
 *  | MultiplicativeExpression ADDITIVE_OPERATOR PrimaryExpression
 *  ;
 */
json::value Parser::MultiplicativeExpression() {
  return _BinaryExpression(
      std::bind(&Parser::PrimaryExpression, this),
      "MULTIPLICATIVE_OPERATOR");
}

/**
 * PrimaryExpression
 *  : Literal
 *  | ParenthesizedExpression
 *  | LeftHandSideExpression
 *  ;
 */
json::value Parser::PrimaryExpression() {
  if (this->_isLiteral(this->m_lookahead)) {
    return this->Literal();
  } 

  auto&& type = this->m_lookahead["type"].as_string();

  if (type == "(") {
    return this->ParenthesizedExpression();
  } else {
    return this->LeftHandSideExpression();
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
 * AssignmentOperator
 *  : SIMPLE_ASSIGN
 *  | COMPLEX_ASSIGN
 *  ;
 */
json::value Parser::AssignmentOperator() {
  if (this->m_lookahead["type"].as_string() == "SIMPLE_ASSIGN") {
    return this->_eat("SIMPLE_ASSIGN");
  } else {
    return this->_eat("COMPLEX_ASSIGN");
  }
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

bool Parser::_isAssignmentOperator(const json::value& token) const {
  auto&& type = token.at("type").as_string();

  if (type == "SIMPLE_ASSIGN" ||
      type == "COMPLEX_ASSIGN") {
    return true;
  } else {
    return false;
  }
}

bool Parser::_isLiteral(const json::value& token) const {
  auto&& type = token.at("type").as_string();

  return type == "NUMBER" || type == "STRING";
}

/**
 * Whether the token is an Assignment Target
 */
const json::value& Parser::_checkValidAssignmentTarget(const json::value& expression) const {
  if (expression.at("type").as_string() == "Identifier") {
    return expression;
  } {
    throw Exception("Invalid left-hand side in assignment expression:\n" + expression.to_string());
  }
}

} // namespace letter
