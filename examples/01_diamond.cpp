// 01_diamond.cpp
//
// Basic C4 diamond example.
//
// Timestamped and Audited both extend Object; Document extends both.
// C4 linearization composes them into one Super chain, so Object::save()
// appears only once.

#include <c4/mixins.hpp>

#include <iostream>

template<class Self, class Super>
struct Object : Super {
  void save() { std::cout << "Object::save\n"; }
};

template<class Self, class Super>
struct Timestamped : Super {
  using c4_parents = c4::parents<Object>;

  void save() { std::cout << "Timestamped::save\n"; Super::save(); }
};

template<class Self, class Super>
struct Audited : Super {
  using c4_parents = c4::parents<Object>;

  void save() { std::cout << "Audited::save\n"; Super::save(); }
};

template<class Self, class Super>
struct Document : Super {
  using c4_parents = c4::parents<Timestamped, Audited>;

  void save() { std::cout << "Document::save\n"; Super::save(); }
};

using MyDocument = c4::instantiate<c4_examples::Document>;

int main() {
  MyDocument doc;
  doc.save();

  // Output:
  //
  // Document::save
  // Timestamped::save
  // Audited::save
  // Object::save
}
