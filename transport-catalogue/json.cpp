#include "json.h"

#include <cctype>
#include <stdexcept>

namespace json {

namespace {

void SkipSpaces(const std::string& str, size_t& pos) {
    while (pos < str.size() && std::isspace(static_cast<unsigned char>(str[pos]))) {
        ++pos;
    }
}

Node ParseNode(const std::string& str, size_t& pos);

Node ParseString(const std::string& str, size_t& pos) {
    pos++; // skip opening quote
    std::string result;
    while (pos < str.size() && str[pos] != '\"') {
        if (str[pos] == '\\') {
            pos++;
            if (pos >= str.size()) {
                break;
            }
            char c = str[pos];
            switch (c) {
                case '\"': result.push_back('\"'); break;
                case '\\': result.push_back('\\'); break;
                case '/': result.push_back('/'); break;
                case 'b': result.push_back('\b'); break;
                case 'f': result.push_back('\f'); break;
                case 'n': result.push_back('\n'); break;
                case 'r': result.push_back('\r'); break;
                case 't': result.push_back('\t'); break;
                default: result.push_back(c); break;
            }
            pos++;
        } else {
            result.push_back(str[pos]);
            pos++;
        }
    }
    if (pos < str.size() && str[pos] == '\"') {
        pos++;
    }
    return Node(result);
}

Node ParseNumber(const std::string& str, size_t& pos) {
    size_t start = pos;
    bool is_double = false;
    while (pos < str.size() && (std::isdigit(static_cast<unsigned char>(str[pos])) || str[pos] == '.' ||
                                str[pos] == 'e' || str[pos] == 'E' ||
                                str[pos] == '+' || str[pos] == '-')) {
        if (str[pos] == '.' || str[pos] == 'e' || str[pos] == 'E') {
            is_double = true;
        }
        pos++;
    }
    std::string num_str = str.substr(start, pos - start);
    if (is_double) {
        double val = std::stod(num_str);
        return Node(val);
    } else {
        int val = std::stoi(num_str);
        return Node(val);
    }
}

Node ParseArray(const std::string& str, size_t& pos) {
    pos++; // skip '['
    Node::Array arr;
    SkipSpaces(str, pos);
    if (pos < str.size() && str[pos] == ']') {
        pos++;
        return Node(arr);
    }
    while (true) {
        SkipSpaces(str, pos);
        Node node = ParseNode(str, pos);
        arr.push_back(std::move(node));
        SkipSpaces(str, pos);
        if (pos < str.size() && str[pos] == ',') {
            pos++;
            continue;
        } else if (pos < str.size() && str[pos] == ']') {
            pos++;
            break;
        } else {
            break;
        }
    }
    return Node(arr);
}

Node ParseObject(const std::string& str, size_t& pos) {
    pos++; // skip '{'
    Node::Object obj;
    SkipSpaces(str, pos);
    if (pos < str.size() && str[pos] == '}') {
        pos++;
        return Node(obj);
    }
    while (true) {
        SkipSpaces(str, pos);
        if (pos < str.size() && str[pos] != '\"') {
            break;
        }
        Node key_node = ParseString(str, pos);
        SkipSpaces(str, pos);
        if (pos < str.size() && str[pos] == ':') {
            pos++;
        } else {
            break;
        }
        SkipSpaces(str, pos);
        Node value_node = ParseNode(str, pos);
        obj.emplace(key_node.AsString(), std::move(value_node));
        SkipSpaces(str, pos);
        if (pos < str.size() && str[pos] == ',') {
            pos++;
            continue;
        } else if (pos < str.size() && str[pos] == '}') {
            pos++;
            break;
        } else {
            break;
        }
    }
    return Node(obj);
}

Node ParseNode(const std::string& str, size_t& pos) {
    SkipSpaces(str, pos);
    if (pos >= str.size()) {
        return Node();
    }
    char c = str[pos];
    if (c == 'n') {
        if (str.substr(pos, 4) == "null") {
            pos += 4;
            return Node(nullptr);
        }
        throw std::runtime_error("Invalid JSON: expected null");
    } else if (c == 't') {
        if (str.substr(pos, 4) == "true") {
            pos += 4;
            return Node(true);
        }
        throw std::runtime_error("Invalid JSON: expected true");
    } else if (c == 'f') {
        if (str.substr(pos, 5) == "false") {
            pos += 5;
            return Node(false);
        }
        throw std::runtime_error("Invalid JSON: expected false");
    } else if (c == '\"') {
        return ParseString(str, pos);
    } else if (c == '[') {
        return ParseArray(str, pos);
    } else if (c == '{') {
        return ParseObject(str, pos);
    } else if (std::isdigit(static_cast<unsigned char>(c)) || c == '-') {
        return ParseNumber(str, pos);
    } else {
        throw std::runtime_error("Invalid JSON: unexpected character");
    }
}

} // namespace

Document Parse(const std::string& input) {
    size_t pos = 0;
    Node root = ParseNode(input, pos);
    return root;
}

void Print(const Node& node, std::ostream& out) {
    if (node.IsNull()) {
        out << "null";
    } else if (node.IsBool()) {
        out << (node.AsBool() ? "true" : "false");
    } else if (node.IsInt()) {
        out << node.AsInt();
    } else if (node.IsDouble()) {
        out << node.AsDouble();
    } else if (node.IsString()) {
        out << '\"';
        for (char c : node.AsString()) {
            if (c == '\"') {
                out << "\\\"";
            } else if (c == '\\') {
                out << "\\\\";
            } else if (c == '\n') {
                out << "\\n";
            } else if (c == '\r') {
                out << "\\r";
            } else if (c == '\t') {
                out << "\\t";
            } else {
                out << c;
            }
        }
        out << '\"';
    } else if (node.IsArray()) {
        out << '[';
        const auto& arr = node.AsArray();
        for (size_t i = 0; i < arr.size(); ++i) {
            if (i > 0) {
                out << ',';
            }
            Print(arr[i], out);
        }
        out << ']';
    } else if (node.IsObject()) {
        out << '{';
        const auto& obj = node.AsObject();
        bool first = true;
        for (const auto& [key, value] : obj) {
            if (!first) {
                out << ',';
            }
            first = false;
            out << '\"' << key << '\"' << ':';
            Print(value, out);
        }
        out << '}';
    }
}

} // namespace json
