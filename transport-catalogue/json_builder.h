#pragma once

#include "json.h"

#include <string>
#include <vector>

namespace json {

class Builder;
class DictItemContext;
class KeyItemContext;
class ArrayItemContext;

class BuilderContextBase {
public:
    explicit BuilderContextBase(Builder& builder)
        : builder_(builder) {
    }

protected:
    Builder& builder_;
};

class DictItemContext : public BuilderContextBase {
public:
    using BuilderContextBase::BuilderContextBase;

    KeyItemContext Key(std::string key);
    Builder& EndDict();
};

class KeyItemContext : public BuilderContextBase {
public:
    using BuilderContextBase::BuilderContextBase;

    DictItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
};

class ArrayItemContext : public BuilderContextBase {
public:
    using BuilderContextBase::BuilderContextBase;

    ArrayItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
};

class Builder {
public:
    Builder();

    KeyItemContext Key(std::string key);
    Builder& Value(Node::Value value);

    DictItemContext StartDict();
    Builder& EndDict();

    ArrayItemContext StartArray();
    Builder& EndArray();

    Node Build();

private:
    Node root_;
    std::vector<Node*> nodes_stack_;

    Node& AddValue(Node node);
};

}  // namespace json
