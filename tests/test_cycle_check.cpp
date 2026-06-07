#include <c4/cycle_check.hpp>

struct A;
struct B;
struct C;
struct D;

struct test_linearization_traits {
  template<class Node>
  using parent_orders = typename Node::parents;

  template<class Node>
  static constexpr bool suffix = Node::suffix;
};

struct A {
  using parents = c4::type_list<>;
  static constexpr bool suffix = false;
};

struct B {
  using parents = c4::type_list<c4::type_list<A>>;
  static constexpr bool suffix = false;
};

struct C {
  using parents = c4::type_list<c4::type_list<B>>;
  static constexpr bool suffix = false;
};

struct D {
  using parents = c4::type_list<c4::type_list<D>>;
  static constexpr bool suffix = false;
};

int main() {
  static_assert(!c4::has_cycle_v<A, test_linearization_traits>);
  static_assert(!c4::has_cycle_v<B, test_linearization_traits>);
  static_assert(!c4::has_cycle_v<C, test_linearization_traits>);
  static_assert(c4::has_cycle_v<D, test_linearization_traits>);
}
