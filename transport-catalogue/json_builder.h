#pragma once

#include <string>
#include <vector>

#include "json.h"

namespace json {

class Builder;
class DictItemContext;
class KeyItemContext;
class ArrayItemContext;

class BuilderContextBase {
  public:
    explicit BuilderContextBase(Builder& builder) : builder_(builder) {
    }

    KeyItemContext Key(std::string key);
    Builder& Value(Node::Value value);

    DictItemContext StartDict();
    Builder& EndDict();

    ArrayItemContext StartArray();
    Builder& EndArray();

    Node Build();

  protected:
    Builder& builder_;
};

class DictItemContext : public BuilderContextBase {
  public:
    using BuilderContextBase::BuilderContextBase;

    Builder& Value(Node::Value value) = delete;
    DictItemContext StartDict() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
    Node Build() = delete;
};

class KeyItemContext : public BuilderContextBase {
  public:
    using BuilderContextBase::BuilderContextBase;

    DictItemContext Value(Node::Value value);

    KeyItemContext Key(std::string key) = delete;
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
    Node Build() = delete;
};

class ArrayItemContext : public BuilderContextBase {
  public:
    using BuilderContextBase::BuilderContextBase;

    ArrayItemContext Value(Node::Value value);

    KeyItemContext Key(std::string key) = delete;
    Builder& EndDict() = delete;
    Node Build() = delete;
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

} // namespace json