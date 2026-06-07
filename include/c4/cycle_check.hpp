#pragma once

#include <c4/type_list.hpp>

namespace c4 {

// ============================================================================
// cycle_check - early diagnostic for cyclic parent graphs
// ============================================================================
//
// Traits must provide:
//   template<class Node> using parent_orders = type_list<type_list<Parent...>, ...>;

template<typename Node, typename Traits, typename Visiting = type_list<>>
struct has_cycle;

template<typename Parents, typename Traits, typename Visiting>
struct any_parent_has_cycle;

template<typename Traits, typename Visiting>
struct any_parent_has_cycle<type_list<>, Traits, Visiting> {
  static constexpr bool value = false;
};

template<typename Parent, typename... Rest, typename Traits, typename Visiting>
struct any_parent_has_cycle<type_list<Parent, Rest...>, Traits, Visiting> {
  static constexpr bool value =
      has_cycle<Parent, Traits, Visiting>::value ||
      any_parent_has_cycle<type_list<Rest...>, Traits, Visiting>::value;
};

template<typename Node, typename Traits, typename Visiting>
struct has_cycle {
  static constexpr bool value =
      contains_v<Visiting, Node> ||
      any_parent_has_cycle<
          typename Traits::template direct_parents<Node>,
          Traits,
          cons_t<Node, Visiting>>::value;
};

template<typename Node, typename Traits>
inline constexpr bool has_cycle_v = has_cycle<Node, Traits>::value;

} // namespace c4
