#include <c4/mixins.hpp>

#include "name_trace.hpp"

#include <cassert>
#include <string_view>
#include <vector>
#include <iostream>

#define C4_NODE(Name, ...)                                                   \
  template<class Self, class Super>                                          \
  struct Name : Super {                                                      \
    using c4_parents = c4::parents<__VA_ARGS__>;                             \
                                                                             \
    void trace_names(std::vector<std::string_view>& names) const {           \
      names.push_back(#Name);                                                \
      Super::trace_names(names);                                             \
    }                                                                        \
  }

#define C4_SUFFIX_NODE(Name, ...)                                            \
  template<class Self, class Super>                                          \
  struct Name : Super {                                                      \
    using c4_parents = c4::parents<__VA_ARGS__>;                             \
    static constexpr bool c4_suffix = true;                                  \
                                                                             \
    void trace_names(std::vector<std::string_view>& names) const {           \
      names.push_back(#Name);                                                \
      Super::trace_names(names);                                             \
    }                                                                        \
  }

template<class T>
std::vector<std::string_view> trace_names() {
  T object;
  std::vector<std::string_view> names;
  object.trace_names(names);
  return names;
}

#define CHECK_CPL(Spec, ...)                                                 \
  do {                                                                       \
    using concrete = c4::instantiate<Spec, name_trace_base>;                 \
    const std::vector<std::string_view> expected = {__VA_ARGS__};            \
    const auto actual = trace_names<concrete>();                             \
    if (actual != expected) {                                                \
      std::cerr << "CPL mismatch for " #Spec "\nexpected:";                  \
      for (auto name : expected) std::cerr << " " << name;                   \
      std::cerr << "\nactual:  ";                                            \
      for (auto name : actual) std::cerr << " " << name;                     \
      std::cerr << "\n";                                                     \
      assert(actual == expected);                                            \
    }                                                                        \
  } while (false)

// --------------------------------------------------------------------------
// Gerbil C4 positive test vectors, ported from src/gerbil/test/c3-test.ss.
// These are end-to-end mixin tests: c4_parents -> linearization -> instantiate.
// --------------------------------------------------------------------------

// Simple roots.
C4_NODE(O);
C4_NODE(A, O);
C4_NODE(B, O);
C4_NODE(C, O);
C4_NODE(D, O);
C4_NODE(E, O);

// Classic C3 examples.
C4_NODE(K1, A, B, C);
C4_NODE(K2, D, B, E);
C4_NODE(K3, D, A);
C4_NODE(Z, K1, K2, K3);

C4_NODE(J1, C, A, B);
C4_NODE(J2, B, D, E);
C4_NODE(J3, A, D);
C4_NODE(Y, J1, J3, J2);

// C3 paper boat/pedalo example.
C4_NODE(DB, B);
C4_NODE(WB, B);
C4_NODE(EL, DB);
C4_NODE(SM, DB);
C4_NODE(PWB, EL, WB);
C4_NODE(SC, SM);
C4_NODE(P, PWB, SC);

// C3 paper grid-layout examples, excluding the inconsistent confused-grid.
C4_NODE(GL, O);
C4_NODE(HG, GL);
C4_NODE(VG, GL);
C4_NODE(HVG, HG, VG);
C4_NODE(VHG, VG, HG);

// StackOverflow MRO example.
C4_NODE(HH);
C4_NODE(GG, HH);
C4_NODE(II, GG);
C4_NODE(FF, HH);
C4_NODE(EE, HH);
C4_NODE(DD, FF);
C4_NODE(CC, EE, FF, GG);
C4_NODE(BB);
C4_NODE(AA, BB, CC, DD);

// C4 suffix examples: lowercase names are suffix classes in the Gerbil tests.
C4_SUFFIX_NODE(o, O);
C4_SUFFIX_NODE(a, o);
C4_SUFFIX_NODE(b, a);
C4_SUFFIX_NODE(c, b, o);
C4_SUFFIX_NODE(d, D, c);

C4_NODE(M, A, B, b, a);
C4_NODE(N, C, c);
C4_NODE(L, M, N);
C4_SUFFIX_NODE(k, D, L);
C4_SUFFIX_NODE(j, E, k, A);
C4_NODE(I, N, M);

// Regression test for bug #1328: non-simultaneous null? cases.
C4_NODE(x1);
C4_NODE(x2, x1);
C4_NODE(x3, x2);
C4_NODE(x4, x3);
C4_NODE(x5, x4, x1);

// Suffix support: cases that preserve the suffix.
C4_NODE(SBA);
C4_NODE(SBB);
C4_NODE(SBS, SBA);
C4_SUFFIX_NODE(sBs, SBA);
C4_NODE(SBC, SBS, SBB);

// Compiler-bootstrap regression case.
C4_SUFFIX_NODE(t);
C4_SUFFIX_NODE(object, t);
C4_SUFFIX_NODE(type, object);
C4_SUFFIX_NODE(procedure, type);
C4_NODE(Primitive, object);
C4_SUFFIX_NODE(primitive_predicate, Primitive, procedure);

int main() {
  CHECK_CPL(O, "O");
  CHECK_CPL(A, "A", "O");
  CHECK_CPL(B, "B", "O");
  CHECK_CPL(C, "C", "O");
  CHECK_CPL(D, "D", "O");
  CHECK_CPL(E, "E", "O");

  CHECK_CPL(K1, "K1", "A", "B", "C", "O");
  CHECK_CPL(K2, "K2", "D", "B", "E", "O");
  CHECK_CPL(K3, "K3", "D", "A", "O");
  CHECK_CPL(Z, "Z", "K1", "K2", "K3", "D", "A", "B", "C", "E", "O");

  CHECK_CPL(J1, "J1", "C", "A", "B", "O");
  CHECK_CPL(J2, "J2", "B", "D", "E", "O");
  CHECK_CPL(J3, "J3", "A", "D", "O");
  CHECK_CPL(Y, "Y", "J1", "C", "J3", "A", "J2", "B", "D", "E", "O");

  CHECK_CPL(DB, "DB", "B", "O");
  CHECK_CPL(WB, "WB", "B", "O");
  CHECK_CPL(EL, "EL", "DB", "B", "O");
  CHECK_CPL(SM, "SM", "DB", "B", "O");
  CHECK_CPL(PWB, "PWB", "EL", "DB", "WB", "B", "O");
  CHECK_CPL(SC, "SC", "SM", "DB", "B", "O");
  CHECK_CPL(P, "P", "PWB", "EL", "SC", "SM", "DB", "WB", "B", "O");

  CHECK_CPL(GL, "GL", "O");
  CHECK_CPL(HG, "HG", "GL", "O");
  CHECK_CPL(VG, "VG", "GL", "O");
  CHECK_CPL(HVG, "HVG", "HG", "VG", "GL", "O");
  CHECK_CPL(VHG, "VHG", "VG", "HG", "GL", "O");

  CHECK_CPL(HH, "HH");
  CHECK_CPL(GG, "GG", "HH");
  CHECK_CPL(II, "II", "GG", "HH");
  CHECK_CPL(FF, "FF", "HH");
  CHECK_CPL(EE, "EE", "HH");
  CHECK_CPL(DD, "DD", "FF", "HH");
  CHECK_CPL(CC, "CC", "EE", "FF", "GG", "HH");
  CHECK_CPL(BB, "BB");
  CHECK_CPL(AA, "AA", "BB", "CC", "EE", "DD", "FF", "GG", "HH");

  CHECK_CPL(o, "o", "O");
  CHECK_CPL(a, "a", "o", "O");
  CHECK_CPL(b, "b", "a", "o", "O");
  CHECK_CPL(c, "c", "b", "a", "o", "O");
  CHECK_CPL(d, "d", "D", "c", "b", "a", "o", "O");

  CHECK_CPL(M, "M", "A", "B", "b", "a", "o", "O");
  CHECK_CPL(N, "N", "C", "c", "b", "a", "o", "O");
  CHECK_CPL(L, "L", "M", "A", "B", "N", "C", "c", "b", "a", "o", "O");
  CHECK_CPL(k, "k", "D", "L", "M", "A", "B", "N", "C", "c", "b", "a", "o", "O");
  CHECK_CPL(j, "j", "E", "k", "D", "L", "M", "A", "B", "N", "C", "c", "b", "a", "o", "O");
  CHECK_CPL(I, "I", "N", "C", "M", "A", "B", "c", "b", "a", "o", "O");

  CHECK_CPL(x1, "x1");
  CHECK_CPL(x2, "x2", "x1");
  CHECK_CPL(x3, "x3", "x2", "x1");
  CHECK_CPL(x4, "x4", "x3", "x2", "x1");
  CHECK_CPL(x5, "x5", "x4", "x3", "x2", "x1");

  CHECK_CPL(SBA, "SBA");
  CHECK_CPL(SBB, "SBB");
  CHECK_CPL(SBS, "SBS", "SBA");
  CHECK_CPL(sBs, "sBs", "SBA");
  CHECK_CPL(SBC, "SBC", "SBS", "SBA", "SBB");

  CHECK_CPL(t, "t");
  CHECK_CPL(object, "object", "t");
  CHECK_CPL(type, "type", "object", "t");
  CHECK_CPL(procedure, "procedure", "type", "object", "t");
  CHECK_CPL(Primitive, "Primitive", "object", "t");
  CHECK_CPL(primitive_predicate, "primitive_predicate", "Primitive", "procedure", "type", "object", "t");
}

#undef CHECK_CPL
#undef C4_SUFFIX_NODE
#undef C4_NODE
