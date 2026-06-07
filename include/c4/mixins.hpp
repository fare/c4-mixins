#pragma once

#include <c4/type_list.hpp>
#include <c4/linearize.hpp>

namespace c4 {

// ============================================================================
// Public parent declarations
// ============================================================================
//
// Common case: one local parent order.
//
//   using c4_parents = c4::parents<A, B, C>;
//
// Advanced case: several local parent orders.
//
//   using c4_parents = c4::parent_orders<
//     c4::order<A, B>,
//     c4::order<C, D>
//   >;

template<template<typename, typename> class... Specs>
struct order {
    // The ordered mixin specs are preserved as template arguments to order.
    // mixin_linearization_traits recovers them by partial specialization.
};

template<typename... Orders>
using parent_orders = type_list<Orders...>;

template<template<typename, typename> class... Specs>
using parents = parent_orders<order<Specs...>>;

// ============================================================================
// mixin - default bottom base and default C4 declarations
// ============================================================================
//
// User mixins inherit from their Super parameter. During metadata extraction we
// pseudo-instantiate each spec as Spec<mixin, mixin>, so omitted declarations are
// inherited from this base.

struct mixin {
    using c4_parents = parents<>;
    static constexpr bool c4_suffix = false;
};

// ============================================================================
// spec_info - metadata node for the generic linearizer
// ============================================================================
//
// spec_info<Spec> adapts a user-written mixin template to the node protocol
// consumed by linearize.hpp. It is not part of the intended user API.

template<template<typename, typename> class Spec>
struct spec_info : mixin {
    // Probe the spec with the default C4 mixin base to read inherited protocol
    // defaults without constructing the final CRTP fixed point.
    using pseudo_instance = Spec<mixin, mixin>;

    using c4_parents = typename pseudo_instance::c4_parents;
    static constexpr bool c4_suffix = pseudo_instance::c4_suffix;

    template<typename Self, typename Super>
    using c4_apply_mixin = Spec<Self, Super>;
};

// ============================================================================
// mixin_linearization_traits - adapter for linearize.hpp
// ============================================================================
//
// linearize.hpp expects parent orders as type_list<type_list<Node...>, ...>.
// User mixins declare c4::parent_orders<c4::order<Spec...>, ...>; this adapter
// turns those public declarations into spec_info nodes for the generic algorithm.

template<typename Order>
struct mixin_order_to_nodes;

template<template<typename, typename> class... Specs>
struct mixin_order_to_nodes<order<Specs...>> {
    using type = type_list<spec_info<Specs>...>;
};

template<typename ParentOrders>
struct mixin_parent_orders_to_nodes;

template<typename... Orders>
struct mixin_parent_orders_to_nodes<type_list<Orders...>> {
    using type = type_list<typename mixin_order_to_nodes<Orders>::type...>;
};

template<typename ParentOrders>
using mixin_parent_orders_to_nodes_t = typename mixin_parent_orders_to_nodes<ParentOrders>::type;

struct mixin_linearization_traits {
    template<typename Node>
    using parent_orders = mixin_parent_orders_to_nodes_t<typename Node::c4_parents>;

    template<typename Node>
    static constexpr bool suffix = Node::c4_suffix;
};

template<typename Node>
using mixin_linearization = linearize<Node, mixin_linearization_traits>;

template<typename Node>
using mixin_precedence_list_t = typename mixin_linearization<Node>::precedence_list;

template<typename Node>
using mixin_most_specific_suffix_t = typename mixin_linearization<Node>::most_specific_suffix;

// ============================================================================
// instantiate - build the concrete C++ inheritance chain
// ============================================================================
//
// The precedence list is [MostSpecific, ..., MostGeneral]. instantiate_chain
// recursively builds:
//
//   MostSpecific<Self, ...<MostGeneral<Self, Base>>>
//
// Every mixin, including suffix classes, receives the same final Self type.

template<typename Self, typename Base, typename PrecedenceList>
struct instantiate_chain;

template<typename Self, typename Base>
struct instantiate_chain<Self, Base, type_list<>> {
    using type = Base;
};

template<typename Self, typename Base, typename Info, typename... Rest>
struct instantiate_chain<Self, Base, type_list<Info, Rest...>> {
    using super = typename instantiate_chain<Self, Base, type_list<Rest...>>::type;

    using type = typename Info::template c4_apply_mixin<Self, super>;
};

// Forward declaration so instantiate<Spec, Base> can be passed as Self while
// computing its own base chain.
template<template<typename, typename> class Spec, typename Base = mixin>
struct instantiate;

template<template<typename, typename> class Spec, typename Base>
struct instantiate_base {
    using node = spec_info<Spec>;
    using linearization = mixin_linearization<node>;
    using precedence_list = typename linearization::precedence_list;
    using inherited_suffix = typename linearization::inherited_suffix;
    using most_specific_suffix = typename linearization::most_specific_suffix;
    using chain_base = typename instantiate_chain<
        instantiate<Spec, Base>,
        Base,
        precedence_list>::type;
};

template<template<typename, typename> class Spec, typename Base>
struct instantiate : instantiate_base<Spec, Base>::chain_base {
    using c4_linearization = typename instantiate_base<Spec, Base>::linearization;
    using c4_precedence_list = typename instantiate_base<Spec, Base>::precedence_list;
    using c4_inherited_suffix = typename instantiate_base<Spec, Base>::inherited_suffix;
    using c4_most_specific_suffix = typename instantiate_base<Spec, Base>::most_specific_suffix;

    using super = typename instantiate_base<Spec, Base>::chain_base;
    using super::super;
};

// Algorithm-name alias retained while the API is still settling.
template<template<typename, typename> class Spec, typename Base = mixin>
using C4 = instantiate<Spec, Base>;

// ============================================================================
// Precedence-list membership checking
// ============================================================================

template<template<typename, typename> class Derived,
         template<typename, typename> class Target>
struct is_in_precedence_list {
    using derived_node = spec_info<Derived>;
    using target_node = spec_info<Target>;
    using precedence_list = mixin_precedence_list_t<derived_node>;

    static constexpr bool value = contains_v<precedence_list, target_node>;
};

template<template<typename, typename> class Derived,
         template<typename, typename> class Target>
inline constexpr bool is_in_precedence_list_v = is_in_precedence_list<Derived, Target>::value;

// Backward-compatible spelling while the library API is still settling.
template<template<typename, typename> class Derived,
         template<typename, typename> class Target>
using is_in_cpl = is_in_precedence_list<Derived, Target>;

template<template<typename, typename> class Derived,
         template<typename, typename> class Target>
inline constexpr bool is_in_cpl_v = is_in_precedence_list_v<Derived, Target>;

} // namespace c4

// ============================================================================
// Version information
// ============================================================================

#define C4_MIXINS_VERSION_MAJOR 0
#define C4_MIXINS_VERSION_MINOR 1
#define C4_MIXINS_VERSION_PATCH 1
#define C4_MIXINS_VERSION "0.1.1"

