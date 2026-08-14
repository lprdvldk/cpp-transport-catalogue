#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;

using Array = std::vector<Node>;
using Object = std::map<std::string, Node>;

class Node : private std::variant<std::nullptr_t, bool, int, double, std::string, Array, Object> {
public:
    using Value = variant;
    using variant::variant;

    Node(const char* value) : variant(std::string(value)) {}

    using Object = json::Object;
    using Array = json::Array;

    bool IsNull() const { return std::holds_alternative<std::nullptr_t>(GetValue()); }
    bool IsBool() const { return std::holds_alternative<bool>(GetValue()); }
    bool IsInt() const { return std::holds_alternative<int>(GetValue()); }
    bool IsDouble() const { return std::holds_alternative<double>(GetValue()); }
    bool IsString() const { return std::holds_alternative<std::string>(GetValue()); }
    bool IsArray() const { return std::holds_alternative<Array>(GetValue()); }
    bool IsObject() const { return std::holds_alternative<Object>(GetValue()); }

    bool AsBool() const { return GetOr<bool>(false); }
    int AsInt() const { return GetOr<int>(0); }
    double AsDouble() const { return GetOr<double>(0.0); }
    const std::string& AsString() const { return GetOrRef<std::string>(); }
    const Array& AsArray() const { return GetOrRef<Array>(); }
    const Object& AsObject() const { return GetOrRef<Object>(); }

    double AsNumber() const {
        if (IsInt()) {
            return AsInt();
        }
        if (IsDouble()) {
            return AsDouble();
        }
        return 0.0;
    }

    const Value& GetValue() const { return *this; }

private:
    template <typename T>
    T GetOr(T default_value) const {
        const T* value = std::get_if<T>(&GetValue());
        return value ? *value : default_value;
    }

    template <typename T>
    const T& GetOrRef() const {
        static const T empty{};
        const T* value = std::get_if<T>(&GetValue());
        return value ? *value : empty;
    }
};

using Document = Node;

Document Parse(const std::string& input);
void Print(const Node& node, std::ostream& out);

} // namespace json
