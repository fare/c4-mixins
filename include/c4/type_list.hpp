#pragma once

#include <type_traits>

namespace c4 {

// ============================================================================
// type_list - fundamental compile-time list type
// ============================================================================

template<typename... Types>
struct type_list {
};

// ============================================================================
// Basic operations
// ============================================================================

// head - first element of a non-empty list.
template<typename List>
struct head;

template<typename T, typename... Rest>
struct head<type_list<T, Rest...>> {
    using type = T;
};

template<typename List>
using head_t = typename head<List>::type;

// tail - all elements except the first.
template<typename List>
struct tail;

template<typename T, typename... Rest>
struct tail<type_list<T, Rest...>> {
    using type = type_list<Rest...>;
};

template<>
struct tail<type_list<>> {
    using type = type_list<>;
};

template<typename List>
using tail_t = typename tail<List>::type;

// cons - prepend one element to a list.
template<typename T, typename List>
struct cons;

template<typename T, typename... Types>
struct cons<T, type_list<Types...>> {
    using type = type_list<T, Types...>;
};

template<typename T, typename List>
using cons_t = typename cons<T, List>::type;

// append - append one element to a list.
template<typename List, typename T>
struct append;

template<typename... Types, typename T>
struct append<type_list<Types...>, T> {
    using type = type_list<Types..., T>;
};

template<typename List, typename T>
using append_t = typename append<List, T>::type;

// concat - concatenate two lists.
template<typename Left, typename Right>
struct concat;

template<typename... Left, typename... Right>
struct concat<type_list<Left...>, type_list<Right...>> {
    using type = type_list<Left..., Right...>;
};

template<typename Left, typename Right>
using concat_t = typename concat<Left, Right>::type;

// ============================================================================
// Query operations
// ============================================================================

// is_empty - true iff the list is empty.
template<typename List>
struct is_empty : std::false_type {};

template<>
struct is_empty<type_list<>> : std::true_type {};

template<typename List>
inline constexpr bool is_empty_v = is_empty<List>::value;

// contains - true iff T occurs in List.
template<typename List, typename T>
struct contains;

template<typename T>
struct contains<type_list<>, T> : std::false_type {};

template<typename T, typename... Rest>
struct contains<type_list<T, Rest...>, T> : std::true_type {};

template<typename Head, typename... Rest, typename T>
struct contains<type_list<Head, Rest...>, T> : contains<type_list<Rest...>, T> {};

template<typename List, typename T>
inline constexpr bool contains_v = contains<List, T>::value;

// ============================================================================
// Transform operations
// ============================================================================

// reverse - reverse a list.
template<typename List, typename Acc = type_list<>>
struct reverse;

template<typename Acc>
struct reverse<type_list<>, Acc> {
    using type = Acc;
};

template<typename T, typename... Rest, typename Acc>
struct reverse<type_list<T, Rest...>, Acc> {
    using type = typename reverse<type_list<Rest...>, cons_t<T, Acc>>::type;
};

template<typename List>
using reverse_t = typename reverse<List>::type;

// map - apply a unary metafunction F<T>::type to each element.
template<template<typename> class F, typename List>
struct map;

template<template<typename> class F>
struct map<F, type_list<>> {
    using type = type_list<>;
};

template<template<typename> class F, typename T, typename... Rest>
struct map<F, type_list<T, Rest...>> {
    using type = cons_t<typename F<T>::type, typename map<F, type_list<Rest...>>::type>;
};

template<template<typename> class F, typename List>
using map_t = typename map<F, List>::type;

// ============================================================================
// C4-specific list operations
// ============================================================================

// remove_nulls - remove empty type_lists from a list of type_lists.
template<typename List>
struct remove_nulls;

template<>
struct remove_nulls<type_list<>> {
    using type = type_list<>;
};

template<typename T, typename... Rest>
struct remove_nulls<type_list<T, Rest...>> {
private:
    using rest_cleaned = typename remove_nulls<type_list<Rest...>>::type;

public:
    using type = std::conditional_t<is_empty_v<T>, rest_cleaned, cons_t<T, rest_cleaned>>;
};

template<typename List>
using remove_nulls_t = typename remove_nulls<List>::type;

// append_reverse_until - reverse Src into Dst until Pred<T>::value is true.
// Returns two members:
//   remaining: the unconsumed suffix of Src, starting with the matching element;
//   result:    Dst with the consumed prefix of Src prepended in reverse order.
template<template<typename> class Pred, typename Src, typename Dst>
struct append_reverse_until;

template<template<typename> class Pred, typename Dst>
struct append_reverse_until<Pred, type_list<>, Dst> {
    using remaining = type_list<>;
    using result = Dst;
};

template<template<typename> class Pred, typename T, typename... Rest, typename Dst>
struct append_reverse_until<Pred, type_list<T, Rest...>, Dst> {
private:
    static constexpr bool matches = Pred<T>::value;
    using next = append_reverse_until<Pred, type_list<Rest...>, cons_t<T, Dst>>;

public:
    using remaining = std::conditional_t<matches, type_list<T, Rest...>, typename next::remaining>;
    using result = std::conditional_t<matches, Dst, typename next::result>;
};

// Append all elements of Items to List, skipping elements already present.
template<typename List, typename Items>
struct append_unique_all;

template<typename List>
struct append_unique_all<List, type_list<>> {
    using type = List;
};

template<typename List, typename First, typename... Rest>
struct append_unique_all<List, type_list<First, Rest...>> {
private:
    using with_first = std::conditional_t<
        contains_v<List, First>,
        List,
        append_t<List, First>>;

public:
    using type = typename append_unique_all<with_first, type_list<Rest...>>::type;
};

template<typename List, typename Items>
using append_unique_all_t = typename append_unique_all<List, Items>::type;

// Concatenate a type_list of type_lists by one level, removing duplicates.
//
//   concat_unique_all_t<type_list<type_list<A, B>, type_list<B, C>>>
//     == type_list<A, B, C>
template<typename Lists>
struct concat_unique_all;

template<>
struct concat_unique_all<type_list<>> {
    using type = type_list<>;
};

template<typename First, typename... Rest>
struct concat_unique_all<type_list<First, Rest...>> {
private:
    using rest = typename concat_unique_all<type_list<Rest...>>::type;

public:
    using type = append_unique_all_t<First, rest>;
};

template<typename Lists>
using concat_unique_all_t = typename concat_unique_all<Lists>::type;

// ============================================================================
// Fold operations
// ============================================================================

// fold_left - left fold with a binary metafunction F<Acc, T>::type.
template<template<typename, typename> class F, typename Init, typename List>
struct fold_left;

template<template<typename, typename> class F, typename Init>
struct fold_left<F, Init, type_list<>> {
    using type = Init;
};

template<template<typename, typename> class F, typename Init, typename T, typename... Rest>
struct fold_left<F, Init, type_list<T, Rest...>> {
    using type = typename fold_left<F, typename F<Init, T>::type, type_list<Rest...>>::type;
};

template<template<typename, typename> class F, typename Init, typename List>
using fold_left_t = typename fold_left<F, Init, List>::type;

} // namespace c4
