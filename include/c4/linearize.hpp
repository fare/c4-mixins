#pragma once

#include <c4/type_list.hpp>
#include <c4/type_map.hpp>
#include <c4/cycle_check.hpp>

#include <type_traits>

namespace c4 {

// ============================================================================
// C4 linearization
// ============================================================================
//
// linearize<Node, Traits> is generic over metadata nodes. Traits must provide:
//
//   template<class Node>
//   using parent_orders = type_list<type_list<Parent...>, ...>;
//
//   template<class Node>
//   static constexpr bool suffix = ...;
//
// mixins.hpp adapts user-written mixin templates into nodes satisfying this
// protocol.

struct no_suffix {};

template<typename Node, typename Traits>
struct linearize;

template<typename Node, typename Traits>
using precedence_list_t =
    typename linearize<Node, Traits>::precedence_list;

template<typename Node, typename Traits>
using inherited_suffix_t =
    typename linearize<Node, Traits>::inherited_suffix;

template<typename Node, typename Traits>
using most_specific_suffix_t =
    typename linearize<Node, Traits>::most_specific_suffix;

template<typename Node, typename Traits>
using parent_orders_t =
    typename Traits::template parent_orders<Node>;

template<typename Node, typename Traits>
inline constexpr bool is_suffix_node_v =
    Traits::template suffix<Node>;

template<typename...>
inline constexpr bool dependent_false_v = false;

// ============================================================================
// Parent-order helpers
// ============================================================================

template<typename Node, typename Traits>
using direct_parents_t =
    concat_unique_all_t<parent_orders_t<Node, Traits>>;

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
using parent_precedence_lists_t =
    typename parent_precedence_lists<Parents, Traits>::type;

// ============================================================================
// C3 merge with ancestor-counting optimization
// ============================================================================

// get_all_tails - collect all non-empty tails from a list of candidate lists.
template<typename Lists>
struct get_all_tails;

template<>
struct get_all_tails<type_list<>> {
    using type = type_list<>;
};

template<typename List, typename... Rest>
struct get_all_tails<type_list<List, Rest...>> {
    using this_tail = tail_t<List>;
    using rest_tails = typename get_all_tails<type_list<Rest...>>::type;

    using type = std::conditional_t<
        is_empty_v<this_tail>,
        rest_tails,
        cons_t<this_tail, rest_tails>>;
};

template<typename Lists>
using get_all_tails_t = typename get_all_tails<Lists>::type;

// flatten - concatenate a type_list of type_lists.
template<typename Lists>
struct flatten;

template<>
struct flatten<type_list<>> {
    using type = type_list<>;
};

template<typename List, typename... Rest>
struct flatten<type_list<List, Rest...>> {
    using type = concat_t<List, typename flatten<type_list<Rest...>>::type>;
};

template<typename Lists>
using flatten_t = typename flatten<Lists>::type;

// count_occurrences - count each type in a list into a type_map.
template<typename List, typename Map>
struct count_occurrences;

template<typename Map>
struct count_occurrences<type_list<>, Map> {
    using type = Map;
};

template<typename T, typename... Rest, typename Map>
struct count_occurrences<type_list<T, Rest...>, Map> {
    using type = typename count_occurrences<
        type_list<Rest...>,
        map_increment_t<Map, T>>::type;
};

template<typename Lists>
struct build_ancestor_counts {
    using all_tails = get_all_tails_t<Lists>;
    using all_tail_elements = flatten_t<all_tails>;

    using type = typename count_occurrences<all_tail_elements, type_map<>>::type;
};

template<typename Lists>
using build_ancestor_counts_t =
    typename build_ancestor_counts<Lists>::type;

// try_list - return the first head in Lists whose tail-count is zero.
template<typename Lists, typename Counts>
struct try_list;

template<typename Counts>
struct try_list<type_list<>, Counts> {
    using type = void;
    static constexpr bool found = false;
};

template<typename List, typename... Rest, typename Counts>
struct try_list<type_list<List, Rest...>, Counts> {
    using rest_result = try_list<type_list<Rest...>, Counts>;

    using type = typename rest_result::type;
    static constexpr bool found = rest_result::found;
};

template<typename H, typename... T, typename... Rest, typename Counts>
struct try_list<type_list<type_list<H, T...>, Rest...>, Counts> {
    static constexpr int count = map_get_v<Counts, H, 0>;
    using rest_result = try_list<type_list<Rest...>, Counts>;

    using type = std::conditional_t<(count == 0), H, typename rest_result::type>;
    static constexpr bool found = (count == 0) || rest_result::found;
};

template<typename Lists, typename Counts>
struct select_next {
    using result = try_list<Lists, Counts>;

    static_assert(
        result::found,
        "C3 merge failed: no valid candidate found. "
        "Check for conflicting local precedence orders or monotonicity violations.");

    using type = typename result::type;
};

template<typename Lists, typename Counts>
using select_next_t = typename select_next<Lists, Counts>::type;

// remove_selected_from_lists - remove selected heads and report new heads whose
// tail-count must be decremented.
template<typename List>
struct new_head_after_removal {
    using type = type_list<>;
};

template<typename H, typename... Rest>
struct new_head_after_removal<type_list<H, Rest...>> {
    using type = type_list<H>;
};

template<typename Lists, typename Selected>
struct remove_selected_from_lists;

template<typename Selected>
struct remove_selected_from_lists<type_list<>, Selected> {
    using lists = type_list<>;
    using modified_heads = type_list<>;
};

template<typename List, typename... Rest, typename Selected>
struct remove_selected_from_lists<type_list<List, Rest...>, Selected> {
    using rest_result =
        remove_selected_from_lists<type_list<Rest...>, Selected>;

    using lists = cons_t<List, typename rest_result::lists>;
    using modified_heads = typename rest_result::modified_heads;
};

template<typename H, typename... T, typename... Rest, typename Selected>
struct remove_selected_from_lists<type_list<type_list<H, T...>, Rest...>, Selected> {
    static constexpr bool matches = std::is_same_v<H, Selected>;
    using tail = type_list<T...>;
    using rest_result =
        remove_selected_from_lists<type_list<Rest...>, Selected>;
    using this_list =
        std::conditional_t<matches, tail, type_list<H, T...>>;
    using this_modified_heads =
        std::conditional_t<matches,
                           typename new_head_after_removal<tail>::type,
                           type_list<>>;

    using lists = cons_t<this_list, typename rest_result::lists>;
    using modified_heads =
        concat_t<this_modified_heads, typename rest_result::modified_heads>;
};

template<typename ModifiedHeads, typename Counts>
struct decrement_modified_heads;

template<typename Counts>
struct decrement_modified_heads<type_list<>, Counts> {
    using type = Counts;
};

template<typename H, typename... Rest, typename Counts>
struct decrement_modified_heads<type_list<H, Rest...>, Counts> {
    using type = typename decrement_modified_heads<
        type_list<Rest...>,
        map_decrement_t<Counts, H>>::type;
};

template<typename Lists, typename Counts, typename Selected>
struct remove_selected_and_update {
    using removed = remove_selected_from_lists<Lists, Selected>;

    using lists = typename removed::lists;
    using counts =
        typename decrement_modified_heads<
            typename removed::modified_heads,
            Counts>::type;
};

// c3_merge_loop - repeatedly select eligible heads and remove them.
template<typename Lists, typename Counts, typename Result = type_list<>>
struct c3_merge_loop {
    using cleaned_lists = remove_nulls_t<Lists>;

    struct empty_case {
        using type = Result;
    };

    struct non_empty_case {
        using next = select_next_t<cleaned_lists, Counts>;
        using updated =
            remove_selected_and_update<cleaned_lists, Counts, next>;
        using next_result = append_t<Result, next>;

        using type =
            typename c3_merge_loop<
                typename updated::lists,
                typename updated::counts,
                next_result>::type;
    };

    using type = typename std::conditional_t<
        is_empty_v<cleaned_lists>,
        empty_case,
        non_empty_case>::type;
};

template<typename Lists>
struct c3_merge {
    using cleaned_lists = remove_nulls_t<Lists>;
    using initial_counts = build_ancestor_counts_t<cleaned_lists>;

    using type = typename c3_merge_loop<cleaned_lists, initial_counts>::type;
};

template<typename Lists>
using c3_merge_t = typename c3_merge<Lists>::type;

// ============================================================================
// Suffix operations
// ============================================================================

// suffix_reaches<S, T> is true iff S is T, or following inherited_suffix from
// S eventually reaches T. This is the template analogue of walking the
// super-suffix chain in the Gerbil implementation.
template<typename Suffix, typename Target, typename Traits>
struct suffix_reaches;

template<typename Traits>
struct suffix_reaches<no_suffix, no_suffix, Traits> {
    static constexpr bool value = true;
};

template<typename Target, typename Traits>
struct suffix_reaches<no_suffix, Target, Traits> {
    static constexpr bool value = false;
};

template<typename Suffix, typename Traits>
struct suffix_reaches<Suffix, no_suffix, Traits> {
    static constexpr bool value = false;
};

template<bool Same, typename Suffix, typename Target, typename Traits>
struct suffix_reaches_impl;

template<typename Suffix, typename Target, typename Traits>
struct suffix_reaches_impl<true, Suffix, Target, Traits> {
    static constexpr bool value = true;
};

template<typename Suffix, typename Target, typename Traits>
struct suffix_reaches_impl<false, Suffix, Target, Traits> {
    using next = inherited_suffix_t<Suffix, Traits>;

    static constexpr bool value =
        suffix_reaches<next, Target, Traits>::value;
};

template<typename Suffix, typename Target, typename Traits>
struct suffix_reaches {
    static constexpr bool value =
        suffix_reaches_impl<
            std::is_same_v<Suffix, Target>,
            Suffix,
            Target,
            Traits>::value;
};

template<typename Suffix, typename Target, typename Traits>
inline constexpr bool suffix_reaches_v =
    suffix_reaches<Suffix, Target, Traits>::value;

// merge_two_suffixes - compatible suffixes must be ordered by suffix ancestry.
// The result is the more-specific suffix.
template<typename S1, typename S2, typename Traits>
struct merge_two_suffixes;

template<typename Traits>
struct merge_two_suffixes<no_suffix, no_suffix, Traits> {
    using type = no_suffix;
};

template<typename S2, typename Traits>
struct merge_two_suffixes<no_suffix, S2, Traits> {
    using type = S2;
};

template<typename S1, typename Traits>
struct merge_two_suffixes<S1, no_suffix, Traits> {
    using type = S1;
};

template<typename S, typename Traits>
struct merge_two_suffixes<S, S, Traits> {
    using type = S;
};

template<bool S1ReachesS2,
         bool S2ReachesS1,
         typename S1,
         typename S2,
         typename Traits>
struct merge_two_suffixes_impl;

template<typename S1, typename S2, typename Traits>
struct merge_two_suffixes_impl<true, false, S1, S2, Traits> {
    using type = S1;
};

template<typename S1, typename S2, typename Traits>
struct merge_two_suffixes_impl<false, true, S1, S2, Traits> {
    using type = S2;
};

template<typename S1, typename S2, typename Traits>
struct merge_two_suffixes_impl<true, true, S1, S2, Traits> {
    using type = S1;
};

template<typename S1, typename S2, typename Traits>
struct merge_two_suffixes_impl<false, false, S1, S2, Traits> {
    static_assert(dependent_false_v<S1, S2, Traits>,
                  "C4 linearization failed: incompatible suffix ancestors");
    using type = no_suffix;
};

template<typename S1, typename S2, typename Traits>
struct merge_two_suffixes {
    using type =
        typename merge_two_suffixes_impl<
            suffix_reaches_v<S1, S2, Traits>,
            suffix_reaches_v<S2, S1, Traits>,
            S1,
            S2,
            Traits>::type;
};

template<typename S1, typename S2, typename Traits>
using merge_two_suffixes_t =
    typename merge_two_suffixes<S1, S2, Traits>::type;

template<typename Parents, typename Traits>
struct merge_parent_suffixes;

template<typename Traits>
struct merge_parent_suffixes<type_list<>, Traits> {
    using type = no_suffix;
};

template<typename Parent, typename... Rest, typename Traits>
struct merge_parent_suffixes<type_list<Parent, Rest...>, Traits> {
    using parent_suffix = most_specific_suffix_t<Parent, Traits>;
    using rest_suffix =
        typename merge_parent_suffixes<type_list<Rest...>, Traits>::type;

    using type = merge_two_suffixes_t<parent_suffix, rest_suffix, Traits>;
};

template<typename Parents, typename Traits>
using merge_parent_suffixes_t =
    typename merge_parent_suffixes<Parents, Traits>::type;

// The full precedence-list tail contributed by an inherited suffix.
//
// For suffix S, this is not merely the chain of suffix nodes; it is S's full
// precedence list. Example: if c is suffix, its tail may be c b a o O.
template<typename Suffix, typename Traits>
struct suffix_tail;

template<typename Traits>
struct suffix_tail<no_suffix, Traits> {
    using type = type_list<>;
};

template<typename Suffix, typename Traits>
struct suffix_tail {
    using type = precedence_list_t<Suffix, Traits>;
};

template<typename Suffix, typename Traits>
using suffix_tail_t =
    typename suffix_tail<Suffix, Traits>::type;

// ============================================================================
// Removing inherited suffix tails before merge
// ============================================================================

// Remove every type in ToRemove from List.
template<typename List, typename ToRemove>
struct remove_all;

template<typename ToRemove>
struct remove_all<type_list<>, ToRemove> {
    using type = type_list<>;
};

template<typename Head, typename... Tail, typename ToRemove>
struct remove_all<type_list<Head, Tail...>, ToRemove> {
    using rest = typename remove_all<type_list<Tail...>, ToRemove>::type;

    using type = std::conditional_t<
        contains_v<ToRemove, Head>,
        rest,
        cons_t<Head, rest>>;
};

template<typename List, typename ToRemove>
using remove_all_t =
    typename remove_all<List, ToRemove>::type;

// Apply remove_all to each list in a type_list of lists, dropping empty lists.
template<typename Lists, typename ToRemove>
struct remove_all_from_lists;

template<typename ToRemove>
struct remove_all_from_lists<type_list<>, ToRemove> {
    using type = type_list<>;
};

template<typename FirstList, typename... RestLists, typename ToRemove>
struct remove_all_from_lists<type_list<FirstList, RestLists...>, ToRemove> {
    using first = remove_all_t<FirstList, ToRemove>;
    using rest =
        typename remove_all_from_lists<type_list<RestLists...>, ToRemove>::type;

    using type = std::conditional_t<
        is_empty_v<first>,
        rest,
        cons_t<first, rest>>;
};

template<typename Lists, typename ToRemove>
using remove_all_from_lists_t =
    typename remove_all_from_lists<Lists, ToRemove>::type;

// Validate that no local parent order escapes the inherited suffix tail.
// Once an order reaches the suffix tail, every later element in that order
// must also be in that tail.
template<typename Order, typename SuffixTail, bool InTail = false>
struct local_order_respects_suffix_tail;

template<typename SuffixTail, bool InTail>
struct local_order_respects_suffix_tail<type_list<>, SuffixTail, InTail> {
    static constexpr bool value = true;
};

template<typename Head, typename... Rest, typename SuffixTail>
struct local_order_respects_suffix_tail<type_list<Head, Rest...>, SuffixTail, false> {
    static constexpr bool head_in_tail = contains_v<SuffixTail, Head>;

    static constexpr bool value =
        local_order_respects_suffix_tail<
            type_list<Rest...>,
            SuffixTail,
            head_in_tail>::value;
};

template<typename Head, typename... Rest, typename SuffixTail>
struct local_order_respects_suffix_tail<type_list<Head, Rest...>, SuffixTail, true> {
    static constexpr bool head_in_tail = contains_v<SuffixTail, Head>;

    static constexpr bool value =
        head_in_tail &&
        local_order_respects_suffix_tail<
            type_list<Rest...>,
            SuffixTail,
            true>::value;
};

template<typename Orders, typename SuffixTail>
struct parent_orders_respect_suffix_tail;

template<typename SuffixTail>
struct parent_orders_respect_suffix_tail<type_list<>, SuffixTail> {
    static constexpr bool value = true;
};

template<typename FirstOrder, typename... RestOrders, typename SuffixTail>
struct parent_orders_respect_suffix_tail<type_list<FirstOrder, RestOrders...>, SuffixTail> {
    static constexpr bool value =
        local_order_respects_suffix_tail<FirstOrder, SuffixTail>::value &&
        parent_orders_respect_suffix_tail<
            type_list<RestOrders...>,
            SuffixTail>::value;
};

template<typename Orders, typename SuffixTail>
inline constexpr bool parent_orders_respect_suffix_tail_v =
    parent_orders_respect_suffix_tail<Orders, SuffixTail>::value;

// ============================================================================
// Entry point
// ============================================================================

template<typename Node, typename Traits>
struct linearize {
    static_assert(!has_cycle_v<Node, Traits>,
                  "C4 linearization failed: parent graph contains a cycle");

    using parent_orders =
        remove_nulls_t<parent_orders_t<Node, Traits>>;

    using parents =
        concat_unique_all_t<parent_orders>;

    using parent_plists =
        parent_precedence_lists_t<parents, Traits>;

    // Most-specific suffix inherited from the direct parents, before deciding
    // whether Node itself becomes the new most-specific suffix.
    using inherited_suffix =
        merge_parent_suffixes_t<parents, Traits>;

    using inherited_tail =
        suffix_tail_t<inherited_suffix, Traits>;

    static_assert(
        parent_orders_respect_suffix_tail_v<parent_orders, inherited_tail>,
        "C4 linearization failed: local parent order escapes inherited suffix tail");

    // C4 removes the inherited suffix tail from all candidates, merges the
    // remaining prefixes, then appends the suffix tail exactly once.
    using stripped_parent_plists =
        remove_all_from_lists_t<parent_plists, inherited_tail>;

    using stripped_parent_orders =
        remove_all_from_lists_t<parent_orders, inherited_tail>;

    using candidates =
        concat_t<stripped_parent_plists, stripped_parent_orders>;

    using merged_prefix =
        c3_merge_t<candidates>;

    using most_specific_suffix = std::conditional_t<
        is_suffix_node_v<Node, Traits>,
        Node,
        inherited_suffix>;

    using precedence_list =
        cons_t<Node, concat_t<merged_prefix, inherited_tail>>;
};

template<typename Node, typename Traits>
using linearize_t =
    typename linearize<Node, Traits>::precedence_list;

template<typename Node, typename Traits>
using get_precedence_list_t =
    linearize_t<Node, Traits>;

} // namespace c4
