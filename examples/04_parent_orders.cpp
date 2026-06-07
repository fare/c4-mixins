// 04_parent_orders.cpp
//
// Advanced parent declarations.
//
// Most mixins use c4::parents<A, B, C>, a single local parent order.
// For advanced cases, c4::parent_orders can declare several local orders.

#include <c4/mixins.hpp>

#include <iostream>

template<class Self, class Super>
struct Object : Super {
  void step() { std::cout << "Object\n"; }
};

template<class Self, class Super>
struct A : Super {
  using c4_parents = c4::parents<Object>;

  void step() { std::cout << "A\n"; Super::step(); }
};

template<class Self, class Super>
struct B : Super {
  using c4_parents = c4::parents<Object>;

  void step() { std::cout << "B\n"; Super::step(); }
};

template<class Self, class Super>
struct C : Super {
  using c4_parents = c4::parents<Object>;

  void step() { std::cout << "C\n"; Super::step(); }
};

template<class Self, class Super>
struct Example : Super {
  using c4_parents = c4::parent_orders<
      c4::order<A, B>,
      c4::order<C>
  >;

  void step() { std::cout << "Example\n"; Super::step(); }
};

using MyExample = c4::instantiate<Example>;

int main() {
  MyExample example;
  example.step();
}
