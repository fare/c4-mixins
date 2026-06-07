#include <c4/cycle_check.hpp>

struct A;
struct B;
struct C;
struct D;

struct E;
struct F;
struct G;

struct Diamond;
struct Left;
struct Right;
struct Root;

struct test_linearization_traits {
  template<class Node>
  using parent_orders = typename Node::parents;

  template<class Node>
  static constexpr bool suffix = Node::suffix;
};

// Acyclic chain: C -> B -> A.
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

// Direct self-cycle: D -> D.
struct D {
  using parents = c4::type_list<c4::type_list<D>>;
  static constexpr bool suffix = false;
};

// Indirect cycle: E -> F -> G -> E.
struct E {
  using parents = c4::type_list<c4::type_list<F>>;
  static constexpr bool suffix = false;
};

struct F {
  using parents = c4::type_list<c4::type_list<G>>;
  static constexpr bool suffix = false;
};

struct G {
  using parents = c4::type_list<c4::type_list<E>>;
  static constexpr bool suffix = false;
};

// Acyclic diamond: Diamond -> Left, Right -> Root.
struct Root {
  using parents = c4::type_list<>;
  static constexpr bool suffix = false;
};

struct Left {
  using parents = c4::type_list<c4::type_list<Root>>;
  static constexpr bool suffix = false;
};

struct Right {
  using parents = c4::type_list<c4::type_list<Root>>;
  static constexpr bool suffix = false;
};

struct Diamond {
  using parents = c4::type_list<c4::type_list<Left, Right>>;
  static constexpr bool suffix = false;
};

int main() {
  static_assert(!c4::has_cycle_v<A, test_linearization_traits>);
  static_assert(!c4::has_cycle_v<B, test_linearization_traits>);
  static_assert(!c4::has_cycle_v<C, test_linearization_traits>);

  static_assert(c4::has_cycle_v<D, test_linearization_traits>);

  static_assert(c4::has_cycle_v<E, test_linearization_traits>);
  static_assert(c4::has_cycle_v<F, test_linearization_traits>);
  static_assert(c4::has_cycle_v<G, test_linearization_traits>);

  static_assert(!c4::has_cycle_v<Root, test_linearization_traits>);
  static_assert(!c4::has_cycle_v<Left, test_linearization_traits>);
  static_assert(!c4::has_cycle_v<Right, test_linearization_traits>);
  static_assert(!c4::has_cycle_v<Diamond, test_linearization_traits>);
}
