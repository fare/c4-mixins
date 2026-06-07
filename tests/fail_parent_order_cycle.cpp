// Expected to fail: cyclic local parent-order constraints.
//
// Port of the Gerbil local-order negative case:
//   ((A B) (B C) (C A))
//
// This is not an inheritance cycle. It is a cycle in the local precedence
// constraints: A before B before C before A.

#include <c4/mixins.hpp>

template<class Self, class Super>
struct O : Super {};

template<class Self, class Super>
struct A : Super {
  using c4_parents = c4::parents<O>;
};

template<class Self, class Super>
struct B : Super {
  using c4_parents = c4::parents<O>;
};

template<class Self, class Super>
struct C : Super {
  using c4_parents = c4::parents<O>;
};

template<class Self, class Super>
struct BadOrder : Super {
  using c4_parents = c4::parent_orders<
      c4::order<A, B>,
      c4::order<B, C>,
      c4::order<C, A>
  >;
};

using bad = c4::instantiate<BadOrder>;

int main() {
  bad x;
  (void)x;
}
