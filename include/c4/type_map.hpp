#pragma once

#include <c4/type_list.hpp>

namespace c4 {

// ============================================================================
// type_map - simple compile-time map from types to values
// ============================================================================
//
// This is a small linear type map used by the C4 linearization algorithm.
// Keys are types; values are non-type template arguments, typically counts.
// The representation is deliberately simple: graphs are expected to be small,
// and readable template instantiations are more valuable here than cleverness.

template<typename Key, auto Value>
struct map_entry {
    using key_type = Key;
    static constexpr auto value = Value;
};

template<typename... Entries>
struct type_map {};

// ============================================================================
// map_get - retrieve the value for a key, or Default if absent
// ============================================================================

template<typename Map, typename Key, auto Default = 0>
struct map_get;

template<typename Key, auto Default>
struct map_get<type_map<>, Key, Default> {
    static constexpr auto value = Default;
};

template<typename Key, auto Value, typename... Rest, auto Default>
struct map_get<type_map<map_entry<Key, Value>, Rest...>, Key, Default> {
    static constexpr auto value = Value;
};

template<typename OtherKey, auto Value, typename... Rest, typename Key, auto Default>
struct map_get<type_map<map_entry<OtherKey, Value>, Rest...>, Key, Default>
    : map_get<type_map<Rest...>, Key, Default> {};

template<typename Map, typename Key, auto Default = 0>
inline constexpr auto map_get_v = map_get<Map, Key, Default>::value;

// ============================================================================
// map_insert - add or update a key-value pair
// ============================================================================

template<typename Entry, typename Map>
struct map_prepend;

template<typename Entry, typename... Entries>
struct map_prepend<Entry, type_map<Entries...>> {
    using type = type_map<Entry, Entries...>;
};

template<typename Entry, typename Map>
using map_prepend_t = typename map_prepend<Entry, Map>::type;

template<typename Map, typename Key, auto Value>
struct map_insert;

template<typename Key, auto Value>
struct map_insert<type_map<>, Key, Value> {
    using type = type_map<map_entry<Key, Value>>;
};

template<typename Key, auto OldValue, typename... Rest, auto NewValue>
struct map_insert<type_map<map_entry<Key, OldValue>, Rest...>, Key, NewValue> {
    using type = type_map<map_entry<Key, NewValue>, Rest...>;
};

template<typename OtherKey, auto OtherValue, typename... Rest, typename Key, auto Value>
struct map_insert<type_map<map_entry<OtherKey, OtherValue>, Rest...>, Key, Value> {
    using type = map_prepend_t<
        map_entry<OtherKey, OtherValue>,
        typename map_insert<type_map<Rest...>, Key, Value>::type>;
};

template<typename Map, typename Key, auto Value>
using map_insert_t = typename map_insert<Map, Key, Value>::type;

// ============================================================================
// map_increment / map_decrement - update numeric values
// ============================================================================

template<typename Map, typename Key, auto Delta = 1>
struct map_increment {
    static constexpr auto current = map_get_v<Map, Key, 0>;

    using type = map_insert_t<Map, Key, current + Delta>;
};

template<typename Map, typename Key, auto Delta = 1>
using map_increment_t = typename map_increment<Map, Key, Delta>::type;

template<typename Map, typename Key, auto Delta = 1>
struct map_decrement {
    static constexpr auto current = map_get_v<Map, Key, 0>;

    using type = map_insert_t<Map, Key, current - Delta>;
};

template<typename Map, typename Key, auto Delta = 1>
using map_decrement_t = typename map_decrement<Map, Key, Delta>::type;

} // namespace c4
