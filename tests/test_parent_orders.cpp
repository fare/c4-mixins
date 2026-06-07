#include <c4/mixins.hpp>

#include "name_trace.hpp"

#include <cassert>
#include <string_view>
#include <vector>

template<class Self, class Super>
struct Object : Super {
  void trace_names(std::vector<std::string_view>& names) const {
    names.push_back("Object");
    Super::trace_names(names);
  }
};

template<class Self, class Super>
struct A : Super {
  using c4_parents = c4::parents<Object>;

  void trace_names(std::vector<std::string_view>& names) const {
    names.push_back("A");
    Super::trace_names(names);
  }
};

template<class Self, class Super>
struct B : Super {
  using c4_parents = c4::parents<Object>;

  void trace_names(std::vector<std::string_view>& names) const {
    names.push_back("B");
    Super::trace_names(names);
  }
};

template<class Self, class Super>
struct C : Super {
  using c4_parents = c4::parents<Object>;

  void trace_names(std::vector<std::string_view>& names) const {
    names.push_back("C");
    Super::trace_names(names);
  }
};

template<class Self, class Super>
struct Example : Super {
  using c4_parents = c4::parent_orders<
      c4::order<A, B>,
      c4::order<C>
  >;

  void trace_names(std::vector<std::string_view>& names) const {
    names.push_back("Example");
    Super::trace_names(names);
  }
};

using example_class =
    c4::instantiate<Example, name_trace_base>;

int main() {
  example_class x;

  std::vector<std::string_view> names;
  x.trace_names(names);

  const std::vector<std::string_view> expected = {
      "Example", "A", "B", "C", "Object"};
  assert(names == expected);
}
