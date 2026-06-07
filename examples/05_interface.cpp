// 05_interface.cpp
//
// C4 mixins can still implement ordinary C++ interfaces.
//
// This example shows a generated class that satisfies both a CRTP-style
// interface and a conventional abstract base interface.

#include <c4/mixins.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <type_traits>

template<class Self>
struct Cloneable {
  virtual std::unique_ptr<Self> clone() const = 0;
  virtual ~Cloneable() = default;
};

struct Serializable {
  virtual std::string serialize() const = 0;
  virtual ~Serializable() = default;
};

template<class Self, class Super>
struct CloneableMixin : Cloneable<Self>, Super {
  std::unique_ptr<Self> clone() const override {
    return std::make_unique<Self>(static_cast<const Self&>(*this));
  }
};

template<class Self, class Super>
struct SerializableMixin : Serializable, Super {
  std::string tag = "node";

  std::string serialize() const override {
    return "{\"tag\":\"" + tag + "\"}";
  }
};

template<class Self, class Super>
struct Node : Super {
  using c4_parents = c4::parents<CloneableMixin, SerializableMixin>;

  int value = 0;
};

using MyNode = c4::instantiate<Node>;

static_assert(std::is_base_of_v<Cloneable<MyNode>, MyNode>);
static_assert(std::is_base_of_v<Serializable, MyNode>);

int main() {
  MyNode node;
  node.value = 42;
  node.tag = "answer";

  Serializable* serializable = &node;
  std::cout << serializable->serialize() << "\n";

  Cloneable<MyNode>* cloneable = &node;
  std::unique_ptr<MyNode> copy = cloneable->clone();
  std::cout << copy->serialize() << "\n";
}
