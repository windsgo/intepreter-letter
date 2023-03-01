#pragma once

#include <optional>
#include <string>
#include <memory>

#include <functional>

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
private:
  json::value Program();
  
  json::value StatementList(const std::optional<std::string>& stop_lookahead_tokentype = std::nullopt);

  json::value Statement();
  json::value ExpressionStatement();
  json::value BlockStatement();
  json::value EmptyStatement();

  json::value Expression();
  json::value AssignmentExpression();
  json::value LeftHandSideExpression();
  json::value Identifier();
  json::value _BinaryExpression(std::function<json::value(void)> builder, const Tokenizer::TokenType& operator_token);
  json::value AdditiveExpression();
  json::value MultiplicativeExpression();
  json::value PrimaryExpression();
  json::value ParenthesizedExpression();

  json::value AssignmentOperator();

  json::value Literal();
  json::value StringLiteral(); 
  json::value NumericLiteral();

private:
  json::value _eat(const std::string& token_type);
  bool _isAssignmentOperator(const json::value& token) const ;
  bool _isLiteral(const json::value& token) const ;
  const json::value& _checkValidAssignmentTarget(const json::value& value) const ;
};

} // namespace letter
