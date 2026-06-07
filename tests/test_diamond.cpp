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
struct Timestamped : Super {
  using c4_parents = c4::parents<Object>;

  void trace_names(std::vector<std::string_view>& names) const {
    names.push_back("Timestamped");
    Super::trace_names(names);
  }
};

template<class Self, class Super>
struct Audited : Super {
  using c4_parents = c4::parents<Object>;

  void trace_names(std::vector<std::string_view>& names) const {
    names.push_back("Audited");
    Super::trace_names(names);
  }
};

template<class Self, class Super>
struct Document : Super {
  using c4_parents = c4::parents<Timestamped, Audited>;

  void trace_names(std::vector<std::string_view>& names) const {
    names.push_back("Document");
    Super::trace_names(names);
  }
};

using document_class =
    c4::instantiate<Document, name_trace_base>;

int main() {
  document_class doc;

  std::vector<std::string_view> names;
  doc.trace_names(names);

  const std::vector<std::string_view> expected = {
      "Document", "Timestamped", "Audited", "Object"};
  assert(names == expected);
}
