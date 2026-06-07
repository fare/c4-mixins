// Expected to fail: suffix constraint violation.
//
// Port of the Gerbil SBc negative case:
//   sBs : SBA   (suffix)
//   SBc : sBs SBB
//
// sBs is a suffix class. If SBc inherits from sBs and then SBB, SBB would
// appear after the suffix tail, violating the suffix property.

#include <c4/mixins.hpp>

template<class Self, class Super>
struct SBA : Super {};

template<class Self, class Super>
struct SBB : Super {};

template<class Self, class Super>
struct sBs : Super {
  using c4_parents = c4::parents<SBA>;
  static constexpr bool c4_suffix = true;
};

template<class Self, class Super>
struct SBc : Super {
  using c4_parents = c4::parents<sBs, SBB>;
};

using bad = c4::instantiate<SBc>;

int main() {
  bad x;
  (void)x;
}
