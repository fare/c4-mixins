#include <c4/linearize.hpp>

#include <type_traits>

struct O;
struct A;
struct B;
struct Diamond;

struct test_linearization_traits {
  template<class Node>
  using parent_orders = typename Node::parents;

  template<class Node>
  static constexpr bool suffix = Node::suffix;
};

struct O {
  using parents = c4::type_list<>;
  static constexpr bool suffix = false;
};

struct A {
  using parents = c4::type_list<c4::type_list<O>>;
  static constexpr bool suffix = false;
};

struct B {
  using parents = c4::type_list<c4::type_list<O>>;
  static constexpr bool suffix = false;
};

struct Diamond {
  using parents = c4::type_list<c4::type_list<A, B>>;
  static constexpr bool suffix = false;
};

using diamond_linearization =
    c4::linearize<Diamond, test_linearization_traits>;

static_assert(std::is_same_v<
    typename diamond_linearization::precedence_list,
    c4::type_list<Diamond, A, B, O>>);

int main() {}
