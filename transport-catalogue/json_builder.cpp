#include "json_builder.h"

#include <utility>

namespace json {

using namespace std::literals;

Builder::Builder() {
    nodes_stack_.push_back(&root_);
}

Node& Builder::AddValue(Node node) {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Adding a value to an already completed object is not allowed"s);
    }

    Node* top = nodes_stack_.back();

    if (top->IsArray()) {
        Array& array = std::get<Array>(top->GetValue());
        array.emplace_back(std::move(node));
        return array.back();
    }

    if (top->IsDict()) {
        throw std::logic_error("Key() must be called before a value inside a Dict"s);
    }

    *top = std::move(node);
    Node& ref = *top;
    nodes_stack_.pop_back();
    return ref;
}

KeyItemContext Builder::Key(std::string key) {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Key() called on an already completed object"s);
    }

    Node* top = nodes_stack_.back();
    if (!top->IsDict()) {
        throw std::logic_error("Key() is allowed only right after StartDict() or after a value inside a Dict"s);
    }

    Dict& dict = std::get<Dict>(top->GetValue());
    Node& value_slot = dict[std::move(key)];
    nodes_stack_.push_back(&value_slot);
    return KeyItemContext(*this);
}

Builder& Builder::Value(Node::Value value) {
    Node node = std::visit(
        [](auto&& alternative) -> Node {
            return Node(std::forward<decltype(alternative)>(alternative));
        },
        std::move(value));
    AddValue(std::move(node));
    return *this;
}

DictItemContext Builder::StartDict() {
    Node& ref = AddValue(Node(Dict{}));
    nodes_stack_.push_back(&ref);
    return DictItemContext(*this);
}

ArrayItemContext Builder::StartArray() {
    Node& ref = AddValue(Node(Array{}));
    nodes_stack_.push_back(&ref);
    return ArrayItemContext(*this);
}

Builder& Builder::EndDict() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("EndDict() called on an already completed object"s);
    }

    Node* top = nodes_stack_.back();
    if (!top->IsDict()) {
        throw std::logic_error("EndDict() does not match an open StartDict()"s);
    }

    nodes_stack_.pop_back();
    return *this;
}

Builder& Builder::EndArray() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("EndArray() called on an already completed object"s);
    }

    Node* top = nodes_stack_.back();
    if (!top->IsArray()) {
        throw std::logic_error("EndArray() does not match an open StartArray()"s);
    }

    nodes_stack_.pop_back();
    return *this;
}

Node Builder::Build() {
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Build() called while the object is not complete"s);
    }
    return root_;
}


KeyItemContext DictItemContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder& DictItemContext::EndDict() {
    return builder_.EndDict();
}


DictItemContext KeyItemContext::Value(Node::Value value) {
    return DictItemContext(builder_.Value(std::move(value)));
}

DictItemContext KeyItemContext::StartDict() {
    return builder_.StartDict();
}

ArrayItemContext KeyItemContext::StartArray() {
    return builder_.StartArray();
}

ArrayItemContext ArrayItemContext::Value(Node::Value value) {
    return ArrayItemContext(builder_.Value(std::move(value)));
}

DictItemContext ArrayItemContext::StartDict() {
    return builder_.StartDict();
}

ArrayItemContext ArrayItemContext::StartArray() {
    return builder_.StartArray();
}

Builder& ArrayItemContext::EndArray() {
    return builder_.EndArray();
}

}  // namespace json
