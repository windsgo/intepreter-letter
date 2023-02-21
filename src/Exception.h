#pragma once

#include <exception>
#include <string>

namespace letter {

class Exception : public std::exception { 
private:
  std::string m_details;

public:
  Exception(const std::string& details) : m_details(details) {}

  virtual const char* what() const noexcept override {
    return m_details.c_str();
  }
};

}
