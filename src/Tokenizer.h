#pragma once

#include "json.hpp"
#include <cstddef>
#include <string>
#include <optional>


namespace letter {

class Tokenizer {
private:
  std::string m_string; 
  std::size_t m_cursor;

public:
  using TokenType = std::optional<std::string>;

  Tokenizer(const std::string& string);
  
  Tokenizer();
  
  void init(const std::string& string);

  inline bool hasMoreTokens() { return this->m_cursor < this->m_string.size(); }

  inline bool isEOF() const { return this->m_cursor == this->m_string.size(); }

  json::value getNextToken(); 

};

}; // namespace letter
