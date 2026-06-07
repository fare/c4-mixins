# C4-Mixins

Header-only C++20 mixins with a C4-linearized `Super` chain.

`c4-mixins` composes C++ mixins into a single inheritance chain.
Each mixin method can call `Super::method()`, and
C4 linearization computes which method comes next.

This gives independently written mixins a cooperative protocol across diamonds:
every ancestor's behavior takes effect once and only once, in a consistent order.
Inconsistent inheritance graphs are rejected at compile time.

## Quick example: a diamond

`Timestamped` and `Audited` both extend `Object`; `Document` extends both.
C4 linearization composes them into one `Super` chain,
so each `save()` method is executed once and only once.

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

using MyDocument = c4::instantiate<Document>;

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

The computed class precedence list, which driving the `Super` chain, is:

```text
Document → Timestamped → Audited → Object
```

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

Sounds confusing? Read my book, as listed in the bibliography.

## Project status

Experimental, pre-1.0.
The same algorithm is used in production in other languages.
The implementation and the examples compile and pass tests.
However, the public C++ API may still change.

## Building and testing

This is a header-only library, so you don't need to build anything.

To run the tests, use the following command, that ought not to output any error message,
and include an "ok" line at the end:

```bash
./run-tests.sh
```

## Examples

The examples in `examples/` are numbered in suggested reading order:

- `01_diamond.cpp`: the basic C4 diamond; shared ancestors appear once in the `Super` chain.
- `02_wrapping.cpp`: cooperative before/after method wrapping around `Super::method()`.
- `03_suffix.cpp`: suffix classes and the single-inheritance tail.
- `04_parent_orders.cpp`: advanced multiple parent-order declarations.
- `05_interface.cpp`: using C4 mixins while programming against ordinary C++ interfaces.
- `06_counting.cpp`: a legacy counting example in the mixin-literature style.

Build all examples into `build/examples/` with:
```bash
./run-tests.sh
```

Or build one manually with, e.g.:

```bash
c++ -std=c++20 -Iinclude examples/01_diamond.cpp -o build/examples/01_diamond
```

## Basic API

A mixin is a class template with two parameters:

```cpp
template<class Self, class Super>
struct MyMixin : Super {
  // ...
};
```

`Self` is the final generated class. `Super` is the next class in the
C4-linearized chain.

A mixin declares its direct parents with `c4_parents`:

```cpp
using c4_parents = c4::parents<A, B, C>;
```

If `c4_parents` is omitted, the mixin has no parents.

Instantiate a concrete class with:

```cpp
using MyClass = c4::instantiate<MyMixin>;
```

or, with an explicit base class:

```cpp
using MyClass = c4::instantiate<MyMixin, MyBase>;
```

The explicit base defaults to `c4::mixin`, which also provides the default C4 declarations.

## Advanced parent orders

Most mixins declare a single local parent (total) order:

```cpp
using c4_parents = c4::parents<A, B, C>;
```

For advanced cases, a mixin can declare several local parent (total) orders:

```cpp
using c4_parents = c4::parent_orders<
  c4::order<A, B>,
  c4::order<C, D>
>;
```

Each `c4::order<...>` is a local precedence list. `c4::parent_orders<...>`
combines several such lists into one parent declaration.
Arbitrary partial orders can be specified this way as the closure of total orders.

See `examples/04_parent_orders.cpp`.

## Suffix classes

A mixin may declare itself a suffix class:

```cpp
static constexpr bool c4_suffix = true;
```

Suffix classes are guaranteed to form a single-inheritance tail in descendant
linearizations. In the generated C++ type, each suffix class has a single most-specific suffix
superclass. This preserves the layout and dispatch opportunities associated
with ordinary single inheritance inside that tail.

Suffix classes still receive the final CRTP `Self` type, like other mixins.
When a suffix class does not actually depend on `Self`, different instantiations
may have identical generated code that optimizing compilers and linkers can
sometimes fold. This is an optimization opportunity, not a semantic guarantee:
C++ type identity still distinguishes different template instantiations.

See `examples/03_suffix.cpp`.

For the theory-inclined, suffix specs have the “suffix property”:
their class precedence list is the suffix of any descendent’s class precedence list.
Suffix specs in a given class’s ancestry are always in a total order,
but some infix (i.e. not-suffix) classes in between them might not.
Suffix specs correspond to the notion of “class” in Scala or Ruby, that are in a mutual
single-inheritance structure, as contrasted with infix (non-suffix) specs, that correspond
to the notion of “trait” in Scala, or “module” in Ruby, that are in mutual multiple-inheritance
structure. See also “struct” in Lisp (that have single inheritance) vs “class” in Lisp (that can
have multiple inheritance) and “mixin” (which is an abstract class designed for use in a multiple
inheritance context, except the word “mixin” pre-dates the words “abstract class”).

## Architecture

```text
include/c4/
├── mixins.hpp        # Main header <--- #INCLUDE THIS
├── type_list.hpp     # Compile-time list operations
├── type_map.hpp      # Compile-time associative map
├── cycle_check.hpp   # Diagnostic-only cycle checker
└── linearize.hpp     # C4 linearization algorithm
```

`mixins.hpp` is the header users include, as `#include <c4/mixins.hpp>`.

The rest is helpers used by `mixins.hpp` itself.
Some readers might be interested in the type-level template programming therein.
File `linearize.hpp` itself is a generic form of the C4 linearization algorithm.


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

## Authors

I (François-René Rideau) designed and implemented this code as part of Gerbil Scheme.
I then used Claude Opus 4.5 from Anthropic to translate it to C++ templates.
Claude one-shotted a working but sloppy solution from specification. I subsequently
rewrote and simplified it into half as much code, with a nicer and more powerful API,
again with some help from Claude. Finally, ChatGPT helped me clean it up further.

I also invented the word "flavorful", after Flavors (1979), the first Object System
that did multiple inheritance the right way, which paved the way to CLOS
(the Commont Lisp Object System), and the basic design of which (if not the advanced features)
was later copied by Ruby, Python, Perl, Scala, and more. For complete explanations,
see my book "Lambda the Ultimate Object" https://fare.tunes.org/files/cs/poof/ltuo.html

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
