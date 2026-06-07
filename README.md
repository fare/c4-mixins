# C4-Mixins: Header-only C++20 Mixins with C4 linearized "Super" chain.

c4-mixins is a header-only C++20 library for mixins that cooperate through Super calls;
ancestor order is computed by C4 linearization.

C4 linearization implements **optimal inheritance**: it combines the expressiveness of
**flavorful multiple inheritance** (cooperative multiple inheritance in the style of
Flavors, CLOS, Ruby, Python, Scala), with the performance of single inheritance where needed,
using "suffix" classes.

## Overview

This project implements **Optimal Inheritance** using C++ template metaprogramming, based on:

1. **Flavorful Multiple Inheritance**:
   Multiple parent methods are not lose-lose conflict, but win-win cooperation. They can each
   call the next (Super) method along a linearized class precedence list. No information loss.
   Flavorful multiple inheritance solves the diamond inheritance problem that gave a bad reputation
   to C++ multiple inheritance.

2. **Linearization Consistency**:
   All linearization algorithms since Flavors preserve the **Inheritance Order**, such that
   a descendent appears before its ancestors in the precedence list.
   C4, like New Flavors, CLOS, C3 (and after them Dylan Python, Perl; but unhappily not Ruby, Scala),
   respects the **Local Precedence Order** between user-specified parents;
   furthermore, C4 uniquely allows this order to be a DAG rather than a total order.
   Like C3 (and Dylan, Python, Perl; but not CLOS, Ruby, Scala), C4 also ensures
   the **Monotonicity** of precedence lists, such that an ancestor's precedence list
   is always a sub-order of its descendent's.
   Like C3, C4 also provides **Extended Precedence**, a consistent preference of a class
   and its exclusive ancestors over others placed after it in the local precedence order.
   These consistency properties ensure that, e.g. you can predictably define methods to
   handle locking or memory allocation for you without risking deadlocks or use-after-free.

3. **Performance of Single Inheritance**:
   Classes that declare "static constexpr bool __c4__is_suffix = true;" are *suffix classes*,
   whose class precedence list is guaranteed to be the suffix of that of any descendent,
   enabling all the usual optimizations associated with single inheritance:
   fixed-offset fields, fixed-offset dynamic method dispatch, etc.
   C++ developers can enjoy their zero-cost abstractions where needed, yet also
   use more expressive virtual dynamic dispatch where necessary.

4. **Compile-time resolution of inheritance**:
   using C++ templates, we ensure that all inheritance computations happen at compile-time;
   there is zero runtime overhead to using this library.

Sounds confusing? Read my book in the bibliography.

## Project Status

It works. Tests pass.

It just needs to be packaged and distributed into a C++ library that programmers will use.

## Authors

I (François-René Rideau) designed and implemented this code as part of Gerbil Scheme.
Then I used Claude Opus 4.5 from Anthropic to translate it to C++ templates.
Claude one-shotted a working but sloppy solution from specification. Finally,
I rewrote and simplified it into half as much code, with a nicer and more powerful API
(again with some help from Claude).

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

## Example Usage

`collectNames` is not part of the core library — it lives in `examples/mixin_names.hpp`,
which provides `MixinNames` (a base with the `collectNames` protocol) and `C4N<Spec>`
(shorthand for `C4<Spec, MixinNames>`).

```cpp
#include "mixin_names.hpp"   // provides MixinNames, C4N
using namespace c4;
using c4::examples::C4N;

// Base spec - no parents
template <typename Self, typename Super>
struct O : public Super {
    using __c4__parents = TypeList<>;
    static constexpr bool __c4__is_suffix = false;
    void collectNames(std::vector<std::string>& names) const {
        names.push_back("O"); Super::collectNames(names);
    }
};

// A and B each inherit from O
template <typename Self, typename Super>
struct A : public Super {
    using __c4__parents = TypeList<SpecList<O>>;
    static constexpr bool __c4__is_suffix = false;
    void collectNames(std::vector<std::string>& names) const {
        names.push_back("A"); Super::collectNames(names);
    }
};

template <typename Self, typename Super>
struct B : public Super {
    using __c4__parents = TypeList<SpecList<O>>;
    static constexpr bool __c4__is_suffix = false;
    void collectNames(std::vector<std::string>& names) const {
        names.push_back("B"); Super::collectNames(names);
    }
};

// Diamond inherits from both A and B
template <typename Self, typename Super>
struct Diamond : public Super {
    using __c4__parents = TypeList<SpecList<A, B>>;
    static constexpr bool __c4__is_suffix = false;
    void collectNames(std::vector<std::string>& names) const {
        names.push_back("Diamond"); Super::collectNames(names);
    }
};

// Compose Diamond; CPL computed at compile time: [Diamond, A, B, O]
using Diamond_Class = C4N<Diamond>;

// Compile-time CPL membership checks (no collectNames needed)
static_assert(IsInCPL_v<Diamond, A>);
static_assert(IsInCPL_v<Diamond, B>);
static_assert(IsInCPL_v<Diamond, O>);

int main() {
    Diamond_Class d;
    std::vector<std::string> names;
    d.collectNames(names);
    // names == {"Diamond", "A", "B", "O"}
    // O appears once despite being a shared ancestor — C4 handles it.
}
```

“Suffix property”: suffix specs (marked `static constexpr bool __c4__is_suffix = true`)
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


## Examples

All runnable examples live in `examples/` and build with:

```bash
mkdir -p build
g++ -std=c++20 -Iinclude examples/<name>.cpp -o build/<name> && build/<name>
```

| File | Demonstrates |
|------|-------------|
| `examples/diamond.cpp` | Basic diamond inheritance; canonical C4 usage with `C4N<>` |
| `examples/interface.cpp` | Shows how to inherit from an interface or abstract class |
| `examples/suffix.cpp` | Suffix specs (`__c4__is_suffix = true`), fixed-tail CPL placement |
| `examples/multiple_parent_lists.cpp` | Multiple independent parent lists: `TypeList<SpecList<A,B>, SpecList<C>>` |
| `examples/mixin_names.hpp` | `MixinNames` base and `C4N<Spec>` alias — used by examples and tests that need `collectNames` |
| `examples/counting.hpp` | `Counting` mixin tracking nodes/edges visited; illustrates stateful mixins |

## Architecture

```
include/c4/
├── c4.hpp            # Main header (include this)
├── type_list.hpp     # Compile-time list operations
├── type_map.hpp      # Compile-time associative map (ancestor counting)
├── dag.hpp           # Cycle detection
└── c4_linearize.hpp  # C4 algorithm (included by c4.hpp)
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

1. **Linearization**: Total order extending the DAG partial order
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

## TODO

  * Package and distribute it as a library that C++ programmers might actually use.
    How? Where? I don't know, I don't partake in the C++ ecosystem.

  * Figure out a way to be O(dn) or at least O(dn log n) with some kind of hash-tables
    or sets for the ancestor counting during template processing.
