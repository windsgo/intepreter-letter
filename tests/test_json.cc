#include "json.hpp"
#include <iostream>
#include <string>

bool operator==(const json::value& lhs, const json::value& rhs)
{
  if (lhs.type() != rhs.type()) return false;
  
  // same type
  // number
  if (lhs.is_number()) {
    return lhs.as_double() == rhs.as_double() && lhs.as_integer() == rhs.as_integer();
  }

  // string
  if (lhs.is_string()) {
    return lhs.as_string() == rhs.as_string();
  }

  // bool
  if (lhs.is_boolean()) {
    return lhs.as_boolean() == rhs.as_boolean();
  }
  
  // null
  if (lhs.is_null()) {
    return rhs.is_null();
  }
  
  // array
  if (lhs.is_array()) {
    auto& la = lhs.as_array();
    auto& ra = rhs.as_array();
    
    if (la.size() != ra.size()) return false;
    for (size_t i = 0; i < la.size(); ++i) {
      if (!(la[i] == ra[i])) return false;
    }
  }

  // object
  if (lhs.is_object()) {
    auto& lo = lhs.as_object();
    auto& ro = rhs.as_object();

    for (auto&& li : lo) {
      if (!ro.find(li.first)) return false;

      if (!(li.second == ro.at(li.first))) return false;
    }
  } 

  return true;
}

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
