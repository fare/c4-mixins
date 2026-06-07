#include <c4/mixins.hpp>

#include <cassert>
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

using node_class = c4::instantiate<Node>;

static_assert(std::is_base_of_v<Cloneable<node_class>, node_class>);
static_assert(std::is_base_of_v<Serializable, node_class>);

int main() {
  node_class node;
  node.tag = "answer";
  node.value = 42;

  Serializable* serializable = &node;
  assert(serializable->serialize() == "{\"tag\":\"answer\"}");

  Cloneable<node_class>* cloneable = &node;
  std::unique_ptr<node_class> copy = cloneable->clone();
  assert(copy->serialize() == "{\"tag\":\"answer\"}");
  assert(copy->value == 42);
}
