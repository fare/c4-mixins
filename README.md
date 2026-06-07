# C4-Mixins

Header-only C++20 mixins with a C4-linearized `Super` chain.

`c4-mixins` composes C++ mixins into a single inheritance chain.
Each mixin method can call `Super::method()`, and
C4 linearization computes which method comes next.

This gives independently written mixins a cooperative protocol to define method across diamonds:
every ancestor's behavior takes effect once and only once, in a consistent order.
Inconsistent inheritance graphs are rejected at compile time.

## Quick Example: A Diamond

```cpp
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

using MyDocument = c4::compose<Document>;

int main() {
  MyDocument doc;
  doc.save();
}
```

Output:

```text
Document::save
Timestamped::save
Audited::save
Object::save
```

The computed class precedence list (driving the chain of methods) is:

```text
Document → Timestamped → Audited → Object
```

Although both `Timestamped` and `Audited` extend `Object`, `Object::save()` appears once and only once.
Each mixin cooperates by calling `Super::save()`.

## Why C4?

This project implements **Optimal Inheritance** using C++ template metaprogramming, based on:

1. **Flavorful Multiple Inheritance**:
   Multiple parent methods are not lose-lose conflict, but win-win cooperation. They can each
   call the next (Super) method along a linearized class precedence list. No information loss.
   Flavorful multiple inheritance addresses the diamond inheritance problem that gave a bad reputation
   to C++ multiple inheritance: in the “flavorless” approach of C++, “diamond” situations of
   multiple independent parents with a common ancestor force are a conflict that must be resolved
   by choosing at most one of the parents and reimplementing any effect desired from the other ones.
   In the flavorful approach, all ancestors cooperate and contribute their behavior without conflict.

2. **Linearization Consistency**:
   All linearization algorithms since Flavors preserve the **Inheritance Order**, such that a
   descendent appears before its ancestors in the precedence list. In addition, C4, like
   New Flavors, CLOS, C3 (and after them Dylan Python, Perl; but unhappily not Ruby, Scala),
   respects the **Local Precedence Order** between user-specified parents; furthermore, C4
   uniquely allows this local precedence order to be an arbitrary DAG rather than a total order.
   Like C3 (and Dylan, Python, Perl; but not CLOS, Ruby, Scala), C4 also ensures the
   **Monotonicity** of precedence lists, such that an ancestor's precedence list is always
   a sub-order of its descendent's. Like C3, C4 also provides **Extended Precedence**, a consistent
   preference of a class and its exclusive ancestors over others placed after it in the local
   precedence order. These consistency properties ensure that, you can predictably define methods
   to e.g. handle locking or memory allocation for you without risking deadlocks or use-after-free.

3. **Performance of Single Inheritance**:
   Classes that declare `static constexpr bool c4_suffix = true;` are *suffix classes*,
   whose class precedence list is guaranteed to be the suffix of that of any descendent,
   enabling all the usual optimizations associated with single inheritance:
   fixed-offset fields, fixed-offset dynamic method dispatch, etc.
   C++ developers can keep a single-inheritance tail where layout and dispatch
   predictability matter, while using cooperative multiple inheritance above that
   tail where expressiveness matters.

4. **Compile-time resolution of inheritance**:
   using C++ templates, we ensure that all inheritance computations happen at compile-time;
   there is zero runtime overhead to using this library.

Sounds confusing? Read my book in the bibliography.

## Project Status

Experimental, pre-1.0.
The same algorithm is used in production in other languages.
The implementation and the examples compile and pass tests.
However, the public C++ API may still change.

## Authors

I (François-René Rideau) designed and implemented this code as part of Gerbil Scheme.
Then I used Claude Opus 4.5 from Anthropic to translate it to C++ templates.
Claude one-shotted a working but sloppy solution from specification. Finally,
I rewrote and simplified it into half as much code, with a nicer and more powerful API,
again with some help from Claude. ChatGPT helped me make clean it up further.

I also invented the word "flavorful", after Flavors (1979), the first Object System
that did multiple inheritance the right way, which paved the way to CLOS
(the Commont Lisp Object System), and the basic design of which (if not the advanced features)
was later copied by Ruby, Python, Perl, Scala, and more. For complete explanations,
see my book "Lambda the Ultimate Object" https://fare.tunes.org/files/cs/poof/ltuo.html

## Building and Testing

```bash
mkdir -p build
g++ -std=c++20 -Iinclude tests/test_spec_pattern.cpp   -o build/test_spec_pattern   && build/test_spec_pattern &&
g++ -std=c++20 -Iinclude tests/test_c3_examples.cpp    -o build/test_c3_examples    && build/test_c3_examples &&
g++ -std=c++20 -Iinclude tests/test_c4_suffix.cpp      -o build/test_c4_suffix      && build/test_c4_suffix &&
g++ -std=c++20 -Iinclude tests/test_error_detection.cpp -o build/test_error_detection && build/test_error_detection
```

## Examples

All runnable examples live in `examples/` and build with:

```bash
mkdir -p build
g++ -std=c++20 -Iinclude examples/<name>.cpp -o build/<name> && build/<name>
```

The examples in `examples/` are numbered in suggested reading order:

- `01_diamond.cpp`: the basic C4 diamond; shared ancestors appear once in the `Super` chain.
- `02_wrapping.cpp`: cooperative before/after method wrapping around `Super::method()`.
- `03_suffix.cpp`: suffix classes and the single-inheritance tail.
- `04_parent_orders.cpp`: advanced multiple parent-order declarations.
- `05_interface.cpp`: using C4 mixins while programming against ordinary C++ interfaces.
- `06_counting.cpp`: a legacy counting example in the mixin-literature style.

## Suffix Classes

“Suffix property”: suffix specs, marked `static constexpr bool c4_suffix = true;`
are guaranteed to have their class precedence list as the suffix of
any descendent’s class precedence list, enabling fixed-offset field access.
Suffix specs in a given class’s ancestry are always in a total order,
though some infix (i.e. not-suffix) classes in between them might not.

Suffix specs correspond to the notion of “class” in Scala or Ruby, that are in a mutual
single-inheritance structure, as contrasted with infix (non-suffix) specs, that correspond
to the notion of “trait” in Scala, or “module” in Ruby, that are in mutual multiple-inheritance
structure. See also “struct” in Lisp (that have single inheritance) vs “class” in Lisp (that can
have multiple inheritance) and “mixin” (which is an abstract class designed for use in a multiple
inheritance context, except the word “mixin” pre-dates the words “abstract class”).

In the generated C++ type, this gives suffix classes a linear tail: each suffix class
has a single suffix superclass. This preserves the layout and dispatch opportunities
associated with ordinary single inheritance inside that tail. When suffix classes do not
depend on the CRTP `Self` parameter, compilers and linkers may also be able to fold identical
generated code, but C++ type identity does not require this.

See examples in [`examples/suffix.cpp`](examples/suffix.cpp).

## Multiple Parent Lists

Instead of parents being in a total order with `using c4_parents = c4::parents<A, B, C>;`
you can specify parents in a partial order, with the following,
where each of the `order<...>` specifies a total order:
```cpp
using c4_parents = c4::parent_orders<c4::order<A, B>, c4::order<C, D>>;`
```

See examples in [`examples/multiple_parent_lists.cpp`](examples/multiple_parent_lists.cpp).

## Architecture

```
include/c4/
├── mixins.hpp        # Main header (include this)
├── type_list.hpp     # Compile-time list operations
├── type_map.hpp      # Compile-time associative map
├── cycle_check.hpp   # Diagnostic-only cycle checker
└── linearize.hpp     # C4 linearization algorithm
```

### Type-Level Infrastructure

**TypeList** - Compile-time list with operations:
- Basic: Head, Tail, Cons, Append, Concat
- Transform: Map, Reverse
- Query: Contains
- Special: RemoveNulls, AppendReverseUntil (for C4), FoldLeft

**TypeMap** - Compile-time associative map:
- Get, Insert, Increment, Decrement
- Used for O(dn) ancestor counting in C3/C4

**SpecHelper** - Internal wrapper for a spec in the inheritance DAG:
```cpp
template <template<typename> class Spec, typename ParentsTypeList,
          bool IsSuffix, size_t UniqueId>
struct SpecHelper { /* ... */ };
```

## The C4 Algorithm

C4 extends C3 with support for **suffix specifications**. It enforces six constraints:

1. **Linearization**: Total order extending the partial order of the inheritance DAG
2. **Local Precedence**: Parent order in definitions preserved in precedence list
3. **Monotonicity**: Parent's precedence list is subsequence of child's
4. **Shape Determinism**: Isomorphic DAGs yield isomorphic precedence lists
5. **Extended Precedence**: Within the other constraints, prioritize as per depth-first search
6. **Suffix Property** (C4): Suffix spec's precedence list is suffix to any of its descendents'.

### Complexity

- **Optimized C4**: C4 is O(dn) when using hash-table for ancestor counting; however,
  I use a simple linear map in my C++ templates, which makes it O(dn²) in practice
  (can you even have template-level hash-tables or sets in C++?).
  Contrast with the original C3 being O(d²n²), while most simple class precedence algorithms
  are O(dn) but fail to provide the good properties of C3 and C4.

## Bibliography

**François-René Rideau**. "Gerbil Scheme C4 implementation". 2025.
https://github.com/mighty-gerbils/gerbil .
Latest copy in branch c3-doc. Source files src/gerbil/runtime/c3.ss,
tests in src/gerbil/test/c3-test.ss, docs in doc/reference/gerbil/runtime/c3.md.

**François-René Rideau**. "Lambda, the Ultimate Object". 2026. (Not yet published)
https://fare.tunes.org/files/cs/poof/ltuo.html .
Includes a complete theory of OO. The C4 algorithm is explained in chapter 7.

**Yannis Smaragdakis and Don Batory**. "Mixin-based programming in C++". 2000.
https://www.researchgate.net/publication/2617570_Mixin-Based_Programming_in_C .
In Proc. International Symposium on Generative and Component-Based Software Engineering,
pp. 164–178. doi:10.1007/3-540-44815-2_12.
Explains the basic approach to implementing Mixin inheritance on top of C++ templates
(not quite as modular as Flavorful Multiple Inheritance, but the basic block
on top of which you can build it).
