// 03_suffix.cpp
//
// Suffix classes and the single-inheritance tail.
//
// Suffix classes are ordinary C++ bases in the generated chain, but C4
// requires them to remain in a single-inheritance suffix tail of descendants.

#include <c4/mixins.hpp>

#include <iostream>
#include <string>
#include <string_view>

template<class Self, class Super>
struct Object : Super {
  static constexpr bool c4_suffix = true;

  void describe() const {
    std::cout << "Object\n";
  }
};

template<class Self, class Super>
struct Named : Super {
  using c4_parents = c4::parents<Object>;
  static constexpr bool c4_suffix = true;

  std::string name;

  void describe() const {
    std::cout << "Named: " << name << "\n";
    Super::describe();
  }
};

template<class Self, class Super>
struct Printable : Super {
  using c4_parents = c4::parents<Named>;

  void describe() const {
    std::cout << "Printable\n";
    Super::describe();
  }

  void print() const {
    std::cout << "print " << this->name << "\n";
  }
};

template<class Self, class Super>
struct Loggable : Super {
  using c4_parents = c4::parents<Named>;

  void describe() const {
    std::cout << "Loggable\n";
    Super::describe();
  }

  void log(std::string_view message) const {
    std::cout << "[" << this->name << "] " << message << "\n";
  }
};

template<class Self, class Super>
struct Service : Super {
  using c4_parents = c4::parents<Printable, Loggable>;

  void describe() const {
    std::cout << "Service\n";
    Super::describe();
  }
};

using MyService = c4::instantiate<Service>;

int main() {
  MyService service;
  service.name = "cache";

  service.describe();
  service.print();
  service.log("started");

  // Expected describe() order:
  //
  // Service
  // Printable
  // Loggable
  // Named
  // Object
}
