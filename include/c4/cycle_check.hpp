#pragma once

#include <c4/type_list.hpp>

namespace c4 {

// ============================================================================
// Cycle detection - check for circular parent dependencies
// ============================================================================
//
// This is diagnostic-only: linearization would eventually fail or recurse
// badly on cyclic parent graphs, but this gives a shorter error path.
//
// Traits must provide:
//
//   template<class Node>
//   using parent_orders = ...;
//
// where parent_orders<Node> is a type_list of local parent orders, e.g.
//
//   type_list<type_list<A, B>, type_list<C>>
//
// The cycle checker flattens those orders into a unique direct-parent list.

template<typename Node, typename Traits>
using cycle_direct_parents_t =
    concat_unique_all_t<typename Traits::template parent_orders<Node>>;

template<typename Node, typename Traits, typename Path = type_list<>>
struct has_cycle;

template<typename Parents, typename Traits, typename Path>
struct any_parent_has_cycle;

template<typename Traits, typename Path>
struct any_parent_has_cycle<type_list<>, Traits, Path> {
    static constexpr bool value = false;
};

template<bool FirstHasCycle, typename Rest, typename Traits, typename Path>
struct any_parent_has_cycle_impl;

template<typename Rest, typename Traits, typename Path>
struct any_parent_has_cycle_impl<true, Rest, Traits, Path> {
    static constexpr bool value = true;
};

template<typename Rest, typename Traits, typename Path>
struct any_parent_has_cycle_impl<false, Rest, Traits, Path>
    : any_parent_has_cycle<Rest, Traits, Path> {};

template<typename Parent, typename... Rest, typename Traits, typename Path>
struct any_parent_has_cycle<type_list<Parent, Rest...>, Traits, Path>
    : any_parent_has_cycle_impl<
          has_cycle<Parent, Traits, Path>::value,
          type_list<Rest...>,
          Traits,
          Path> {};

template<bool InPath, typename Node, typename Traits, typename Path>
struct has_cycle_impl;

template<typename Node, typename Traits, typename Path>
struct has_cycle_impl<true, Node, Traits, Path> {
    static constexpr bool value = true;
};

template<typename Node, typename Traits, typename Path>
struct has_cycle_impl<false, Node, Traits, Path> {
private:
    using parents = cycle_direct_parents_t<Node, Traits>;
    using new_path = cons_t<Node, Path>;

public:
    static constexpr bool value =
        any_parent_has_cycle<parents, Traits, new_path>::value;
};

template<typename Node, typename Traits, typename Path>
struct has_cycle
    : has_cycle_impl<contains_v<Path, Node>, Node, Traits, Path> {};

template<typename Node, typename Traits>
inline constexpr bool has_cycle_v = has_cycle<Node, Traits>::value;

} // namespace c4
