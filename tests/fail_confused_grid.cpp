// Expected to fail: confused-grid local precedence inconsistency.
//
// Port of the Gerbil CG negative case:
//   HVG : HG VG
//   VHG : VG HG
//   CG  : HVG VHG
//
// HVG requires HG before VG, while VHG requires VG before HG.

#include <c4/mixins.hpp>

template<class Self, class Super>
struct O : Super {};

template<class Self, class Super>
struct GL : Super {
  using c4_parents = c4::parents<O>;
};

template<class Self, class Super>
struct HG : Super {
  using c4_parents = c4::parents<GL>;
};

template<class Self, class Super>
struct VG : Super {
  using c4_parents = c4::parents<GL>;
};

template<class Self, class Super>
struct HVG : Super {
  using c4_parents = c4::parents<HG, VG>;
};

template<class Self, class Super>
struct VHG : Super {
  using c4_parents = c4::parents<VG, HG>;
};

template<class Self, class Super>
struct CG : Super {
  using c4_parents = c4::parents<HVG, VHG>;
};

using bad = c4::instantiate<CG>;

int main() {
  bad x;
  (void)x;
}
