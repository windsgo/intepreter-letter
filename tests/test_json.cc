#include "json.hpp"
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  /*
  json::value v;
  v["type"] = "NumericLiteral";
  v["value"] = 1;
  std::cout << v << std::endl;
  std::cout << v.as_object() << std::endl;
  std::cout << v.to_string() << std::endl;
  std::cout << (v.type() == json::value::value_type::Object) << std::endl;

  std::string c = "a3";
  std::cout << std::isdigit(c[0]) << std::isdigit(c[1]) << std::endl;
  std::cout << std::stoi(c.substr(1)) << std::endl;
 
  json::object vv = json::object{
    {"type", "ha"},
    {"value", 1}
  };
  std::cout << vv.format() << std::endl;
  // std::cout << vv. << std::endl;
  
  std::cout << "-- test equal -- " << std::endl;
  */
  json::value v1;
  json::value v2;

  v1["type"] = "a";
  v1["value"] = 123;
  v1["arr1"] = json::array({1,2,3});

  v2["value"] = 123;
  v2["arr1"] = json::array({1,2,3});
  v2["type"] = "a";
  

  std::cout << (v1 == v2) << std::endl;
  
  std::cout << v2.format(true) << std::endl;
  std::cout << v1.format(true) << std::endl;
  
  json::value ass1;
  ass1["arr"] = json::array({1,2,3});
  std::cout << ass1.is_array() << ass1.is_object() << std::endl;
  std::cout << ass1["arr"].is_array() << ass1["arr1"].is_object() << std::endl;
  
  json::array arr;
  arr.emplace_back(json::value("aa"));
  arr.emplace_back(json::value(1234));
 
  std::cout << arr << std::endl;

  json::value o;
  json::value expression = json::object{
    {"type", "NumericLiteral"},
    {"value", 789}
  };
  
  std::cout << o.format() << std::endl;
  return 0;
}
