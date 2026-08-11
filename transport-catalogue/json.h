#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node {
public:
  using Object = std::map<std::string, Node>;
  using Array = std::vector<Node>;

  Node() : type_(Type::Null) {}
  Node(std::nullptr_t) : type_(Type::Null) {}
  Node(bool value) : type_(Type::Bool), bool_value_(value) {}
  Node(int value) : type_(Type::Int), int_value_(value) {}
  Node(double value) : type_(Type::Double), double_value_(value) {}
  Node(const std::string &value) : type_(Type::String), string_value_(value) {}
  Node(const char *value) : type_(Type::String), string_value_(value) {}
  Node(const Array &value) : type_(Type::Array), array_value_(value) {}
  Node(const Object &value) : type_(Type::Object), object_value_(value) {}

  bool IsNull() const { return type_ == Type::Null; }
  bool IsBool() const { return type_ == Type::Bool; }
  bool IsInt() const { return type_ == Type::Int; }
  bool IsDouble() const { return type_ == Type::Double; }
  bool IsString() const { return type_ == Type::String; }
  bool IsArray() const { return type_ == Type::Array; }
  bool IsObject() const { return type_ == Type::Object; }

  bool AsBool() const { return bool_value_; }
  int AsInt() const { return int_value_; }
  double AsDouble() const { return double_value_; }
  const std::string &AsString() const { return string_value_; }
  const Array &AsArray() const { return array_value_; }
  const Object &AsObject() const { return object_value_; }

  double AsNumber() const {
    if (type_ == Type::Int) {
      return int_value_;
    }
    if (type_ == Type::Double) {
      return double_value_;
    }
    return 0.0;
  }

private:
  enum class Type { Null, Bool, Int, Double, String, Array, Object };
  Type type_;
  bool bool_value_ = false;
  int int_value_ = 0;
  double double_value_ = 0.0;
  std::string string_value_;
  Array array_value_;
  Object object_value_;
};

using Document = Node;

Document Parse(const std::string &input);
void Print(const Node &node, std::ostream &out);

} // namespace json
