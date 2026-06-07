#pragma once

#include <c4/cycle_check.hpp>
#include <c4/type_list.hpp>
#include <c4/type_map.hpp>

#include <type_traits>

namespace c4 {

// ============================================================================
// linearize - C4 linearization over generic nodes
// ============================================================================
// Traits must provide:
//   template<class Node> using parent_orders = type_list<type_list<Parent...>, ...>;
//   template<class Node> static constexpr bool suffix = ...;
//
// linearize<Node, Traits> computes and caches, as a named template
// specialization, the precedence list and the most-specific suffix ancestor.

struct no_suffix {};

template<typename PrecedenceList, typename InheritedSuffix, typename MostSpecificSuffix>
struct linearization_result {
    using precedence_list = PrecedenceList;
    using inherited_suffix = InheritedSuffix;
    using most_specific_suffix = MostSpecificSuffix;
};

template<typename Node, typename Traits>
struct linearize;

template<typename Node, typename Traits>
using precedence_list_t = typename linearize<Node, Traits>::precedence_list;

template<typename Node, typename Traits>
using inherited_suffix_t = typename linearize<Node, Traits>::inherited_suffix;

template<typename Node, typename Traits>
using most_specific_suffix_t = typename linearize<Node, Traits>::most_specific_suffix;

template<typename Node, typename Traits>
inline constexpr bool is_suffix_node_v = Traits::template suffix<Node>;

// ----------------------------------------------------------------------------
// Small local list utilities used by the merge.
// ----------------------------------------------------------------------------

template<typename T, typename U>
struct same_type : std::false_type {};

template<typename T>
struct same_type<T, T> : std::true_type {};

template<typename List, typename T>
struct append_unique;

template<typename T>
struct append_unique<type_list<>, T> {
    using type = type_list<T>;
};

template<typename... Ts, typename T>
struct append_unique<type_list<Ts...>, T> {
    using type = std::conditional_t<contains_v<type_list<Ts...>, T>,
                                    type_list<Ts...>,
                                    type_list<Ts..., T>>;
};

template<typename List, typename T>
using append_unique_t = typename append_unique<List, T>::type;

template<typename Acc, typename List>
struct unique_append_all;

template<typename Acc>
struct unique_append_all<Acc, type_list<>> {
    using type = Acc;
};

template<typename Acc, typename T, typename... Rest>
struct unique_append_all<Acc, type_list<T, Rest...>> {
    using type = typename unique_append_all<
        append_unique_t<Acc, T>, type_list<Rest...>>::type;
};

template<typename Acc, typename List>
using unique_append_all_t = typename unique_append_all<Acc, List>::type;

// Flatten parent orders, preserving first occurrence of each parent.
template<typename ParentOrders>
struct unique_direct_parents;

template<>
struct unique_direct_parents<type_list<>> {
    using type = type_list<>;
};

template<typename FirstOrder, typename... RestOrders>
struct unique_direct_parents<type_list<FirstOrder, RestOrders...>> {
private:
    using rest = typename unique_direct_parents<type_list<RestOrders...>>::type;
public:
    using type = unique_append_all_t<FirstOrder, rest>;
};

template<typename ParentOrders>
using unique_direct_parents_t = typename unique_direct_parents<ParentOrders>::type;

// ----------------------------------------------------------------------------
// Suffix merging.
// ----------------------------------------------------------------------------

template<typename S, typename Target, typename Traits>
struct suffix_reaches;

template<typename Traits>
struct suffix_reaches<no_suffix, no_suffix, Traits> : std::true_type {};

template<typename Target, typename Traits>
struct suffix_reaches<no_suffix, Target, Traits> : std::false_type {};

template<typename S, typename Traits>
struct suffix_reaches<S, S, Traits> : std::true_type {};

template<typename S, typename Target, typename Traits>
struct suffix_reaches
    : suffix_reaches<inherited_suffix_t<S, Traits>, Target, Traits> {};

template<typename S, typename Target, typename Traits>
inline constexpr bool suffix_reaches_v = suffix_reaches<S, Target, Traits>::value;

template<typename...>
struct dependent_false : std::false_type {};

template<typename S1, typename S2, typename Traits>
struct incompatible_suffixes {
    static_assert(dependent_false<S1, S2, Traits>::value,
                  "C4 linearization failed: incompatible suffix ancestors");
    using type = no_suffix;
};

template<typename S1, typename S2, typename Traits>
struct merge_two_suffixes;

template<typename Traits>
struct merge_two_suffixes<no_suffix, no_suffix, Traits> {
    using type = no_suffix;
};

template<typename S, typename Traits>
struct merge_two_suffixes<S, no_suffix, Traits> {
    using type = S;
};

template<typename S, typename Traits>
struct merge_two_suffixes<no_suffix, S, Traits> {
    using type = S;
};

template<typename S, typename Traits>
struct merge_two_suffixes<S, S, Traits> {
    using type = S;
};

template<typename S1, typename S2, typename Traits>
struct merge_two_suffixes {
    using type = std::conditional_t<
        suffix_reaches_v<S1, S2, Traits>,
        S1,
        std::conditional_t<
            suffix_reaches_v<S2, S1, Traits>,
            S2,
            typename incompatible_suffixes<S1, S2, Traits>::type>>;
};

template<typename S1, typename S2, typename Traits>
using merge_two_suffixes_t = typename merge_two_suffixes<S1, S2, Traits>::type;

template<typename ParentList, typename Traits>
struct merge_parent_suffixes;

template<typename Traits>
struct merge_parent_suffixes<type_list<>, Traits> {
    using type = no_suffix;
};

template<typename Parent, typename... Rest, typename Traits>
struct merge_parent_suffixes<type_list<Parent, Rest...>, Traits> {
private:
    using parent_suffix = most_specific_suffix_t<Parent, Traits>;
    using rest_suffix = typename merge_parent_suffixes<type_list<Rest...>, Traits>::type;
public:
    using type = merge_two_suffixes_t<parent_suffix, rest_suffix, Traits>;
};

template<typename ParentList, typename Traits>
using merge_parent_suffixes_t = typename merge_parent_suffixes<ParentList, Traits>::type;

// ----------------------------------------------------------------------------
// Candidate list construction for C3 merge.
// ----------------------------------------------------------------------------

template<typename Parents, typename Traits>
struct parent_precedence_lists;

template<typename Traits>
struct parent_precedence_lists<type_list<>, Traits> {
    using type = type_list<>;
};

template<typename Parent, typename... Rest, typename Traits>
struct parent_precedence_lists<type_list<Parent, Rest...>, Traits> {
    using type = cons_t<
        precedence_list_t<Parent, Traits>,
        typename parent_precedence_lists<type_list<Rest...>, Traits>::type>;
};

template<typename Parents, typename Traits>
using parent_precedence_lists_t = typename parent_precedence_lists<Parents, Traits>::type;

// ----------------------------------------------------------------------------
// C3 merge.
// ----------------------------------------------------------------------------

template<typename Candidate, typename Lists>
struct appears_in_any_tail;

template<typename Candidate>
struct appears_in_any_tail<Candidate, type_list<>> : std::false_type {};

template<typename Candidate, typename Head, typename... Tail, typename... RestLists>
struct appears_in_any_tail<Candidate, type_list<type_list<Head, Tail...>, RestLists...>>
    : std::conditional_t<
          contains_v<type_list<Tail...>, Candidate>,
          std::true_type,
          appears_in_any_tail<Candidate, type_list<RestLists...>>> {};

template<typename Candidate, typename... RestLists>
struct appears_in_any_tail<Candidate, type_list<type_list<>, RestLists...>>
    : appears_in_any_tail<Candidate, type_list<RestLists...>> {};

template<typename Lists>
struct select_candidate;

struct no_candidate {};

template<>
struct select_candidate<type_list<>> {
    using type = no_candidate;
};

template<typename Head, typename... Tail, typename... RestLists>
struct select_candidate<type_list<type_list<Head, Tail...>, RestLists...>> {
    using type = std::conditional_t<
        appears_in_any_tail<Head, type_list<type_list<Head, Tail...>, RestLists...>>::value,
        typename select_candidate<type_list<RestLists...>>::type,
        Head>;
};

template<typename... RestLists>
struct select_candidate<type_list<type_list<>, RestLists...>>
    : select_candidate<type_list<RestLists...>> {};

template<typename Lists>
using select_candidate_t = typename select_candidate<Lists>::type;

template<typename List, typename Next>
struct pop_if_head;

template<typename Next>
struct pop_if_head<type_list<>, Next> {
    using type = type_list<>;
};

template<typename Next, typename... Rest>
struct pop_if_head<type_list<Next, Rest...>, Next> {
    using type = type_list<Rest...>;
};

template<typename Head, typename... Rest, typename Next>
struct pop_if_head<type_list<Head, Rest...>, Next> {
    using type = type_list<Head, Rest...>;
};

template<typename List, typename Next>
using pop_if_head_t = typename pop_if_head<List, Next>::type;

template<typename Lists, typename Next>
struct remove_next_from_lists;

template<typename Next>
struct remove_next_from_lists<type_list<>, Next> {
    using type = type_list<>;
};

template<typename FirstList, typename... RestLists, typename Next>
struct remove_next_from_lists<type_list<FirstList, RestLists...>, Next> {
private:
    using first = pop_if_head_t<FirstList, Next>;
    using rest = typename remove_next_from_lists<type_list<RestLists...>, Next>::type;
public:
    using type = std::conditional_t<is_empty_v<first>, rest, cons_t<first, rest>>;
};

template<typename Lists, typename Next>
using remove_next_from_lists_t = typename remove_next_from_lists<Lists, Next>::type;

template<typename Candidates, typename Acc>
struct c3_merge_loop;

template<typename Acc>
struct c3_merge_loop<type_list<>, Acc> {
    using type = reverse_t<Acc>;
};

template<typename OnlyList, typename Acc>
struct c3_merge_loop<type_list<OnlyList>, Acc> {
    using type = concat_t<reverse_t<Acc>, OnlyList>;
};

template<typename FirstList, typename SecondList, typename... RestLists, typename Acc>
struct c3_merge_loop<type_list<FirstList, SecondList, RestLists...>, Acc> {
private:
    using candidates = type_list<FirstList, SecondList, RestLists...>;
    using next = select_candidate_t<candidates>;
    using remaining = remove_next_from_lists_t<candidates, next>;
public:
    using type = typename c3_merge_loop<remaining, cons_t<next, Acc>>::type;
};

template<typename Candidates>
using c3_merge_t = typename c3_merge_loop<remove_nulls_t<Candidates>, type_list<>>::type;

// ----------------------------------------------------------------------------
// Main linearization.
// ----------------------------------------------------------------------------

template<typename Node, typename Traits>
struct linearize {
    static_assert(!has_cycle_v<Node, Traits>,
                  "C4 linearization failed: parent graph contains a cycle");

private:
    using parent_orders = remove_nulls_t<typename Traits::template parent_orders<Node>>;
    using parents = concat_unique_all_t<parent_orders>;
    using parent_plists = parent_precedence_lists_t<parents, Traits>;
    using candidates = concat_t<parent_plists, parent_orders>;

public:
    using inherited_suffix = merge_parent_suffixes_t<parents, Traits>;
    using most_specific_suffix = std::conditional_t<
        is_suffix_node_v<Node, Traits>, Node, inherited_suffix>;

    using precedence_list = cons_t<Node, c3_merge_t<candidates>>;
};

template<typename Node, typename Traits>
using linearize_t = typename linearize<Node, Traits>::precedence_list;

} // namespace c4
