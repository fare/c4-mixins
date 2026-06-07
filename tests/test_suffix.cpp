#include <c4/mixins.hpp>

#include "name_trace.hpp"

#include <cassert>
#include <string>
#include <string_view>
#include <vector>

template<class Self, class Super>
struct Object : Super {
  static constexpr bool c4_suffix = true;

  void trace_names(std::vector<std::string_view>& names) const {
    names.push_back("Object");
    Super::trace_names(names);
  }
};

template<class Self, class Super>
struct Named : Super {
  using c4_parents = c4::parents<Object>;
  static constexpr bool c4_suffix = true;

  std::string name;

  void trace_names(std::vector<std::string_view>& names) const {
    names.push_back("Named");
    Super::trace_names(names);
  }
};

template<class Self, class Super>
struct Printable : Super {
  using c4_parents = c4::parents<Named>;

  void trace_names(std::vector<std::string_view>& names) const {
    names.push_back("Printable");
    Super::trace_names(names);
  }
};

template<class Self, class Super>
struct Loggable : Super {
  using c4_parents = c4::parents<Named>;

  void trace_names(std::vector<std::string_view>& names) const {
    names.push_back("Loggable");
    Super::trace_names(names);
  }
};

template<class Self, class Super>
struct Service : Super {
  using c4_parents = c4::parents<Printable, Loggable>;

  void trace_names(std::vector<std::string_view>& names) const {
    names.push_back("Service");
    Super::trace_names(names);
  }
};

using service_class =
    c4::instantiate<Service, name_trace_base>;

int main() {
  service_class svc;

  std::vector<std::string_view> names;
  svc.trace_names(names);

  const std::vector<std::string_view> expected = {
      "Service", "Printable", "Loggable", "Named", "Object"};
  assert(names == expected);
}
