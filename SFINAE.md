# `sfinae.h` Documentation

This document explains `include/sfinae.h` in detail: what each utility does, where it is used, and why `sfinae_bool` exists.

## 1. Purpose of `sfinae.h`

`sfinae.h` provides small compile-time utilities used to:

1. Detect whether certain expressions are valid for a type.
2. Convert those detections into boolean-like type traits.
3. Support generic operations on tuples (zip/for_each helpers).

Main consumers in this codebase are:

1. `include/json.h`
2. `include/stringify.h`
3. `include/user_object.h` (indirectly through stringify/json traits)

## 2. Boolean Carrier Types

```cpp
template<class>
struct sfinae_bool {};

template<class T>
struct sfinae_true : sfinae_bool<T>, std::true_type{};

template<class T>
struct sfinae_false : sfinae_bool<T>, std::false_type{};
```

### What they are

1. `sfinae_true<T>` behaves like `std::true_type`.
2. `sfinae_false<T>` behaves like `std::false_type`.
3. Both also inherit from `sfinae_bool<T>`.

### Why the extra `sfinae_bool<T>` base exists

It is used to implement a custom type-level `AND` operator without writing all combinations manually.

```cpp
template<class T, class S>
sfinae_false<T> operator*(sfinae_bool<T>, sfinae_bool<S>);

template<class T, class S>
sfinae_true<T> operator*(sfinae_true<T>, sfinae_true<S>);
```

How this works:

1. `true * true` matches the second overload exactly and returns `sfinae_true<T>`.
2. Any other combination (`true * false`, `false * true`, `false * false`) falls back to the first overload via base conversion (`sfinae_true/false` -> `sfinae_bool`) and returns `sfinae_false<T>`.

Without `sfinae_bool<T>`, you would need to explicitly provide all non-true/true combinations.

## 3. Detection Helpers (SFINAE Probes)

Inside `detail` namespace, each probe uses two overloads:

1. `int` overload: selected first if expression is valid.
2. `long` overload: fallback when substitution fails.

### Example pattern

```cpp
template<class T>
static auto has_tostring(int) -> sfinae_true<decltype(to_string(std::declval<T>()))>;

template<class T>
static auto has_tostring(long) -> sfinae_false<T>;
```

Interpretation:

1. Try to form `to_string(std::declval<T>())`.
2. If valid, result type is `sfinae_true<...>`.
3. If invalid, the `int` overload is removed by SFINAE and the `long` overload returns `sfinae_false<T>`.

Same structure is used for:

1. `has_std_tostring`: checks `std::to_string(T)`.
2. `has_begin`: checks `obj.begin()`.
3. `has_end`: checks `obj.end()`.

Public traits expose these checks:

```cpp
template<class T>
struct has_tostring : decltype(detail::has_tostring<T>(0)){};

template<class T>
struct has_std_tostring : decltype(detail::has_std_tostring<T>(0)){};

template<class T>
struct has_begin_end : decltype(detail::has_begin<T>(0) * detail::has_end<T>(0)){};
```

`has_begin_end<T>` is exactly where `sfinae_bool` + `operator*` composition is used.

## 4. Where Traits Are Used

## 4.1 In `include/json.h`

Traits select JSON constructors/overloads:

1. Prefer `to_json(T)` if available.
2. Else prefer custom `to_string(T)`.
3. Else prefer `std::to_string(T)`.
4. Else fall back to generic string/object handling.
5. For containers, `has_begin_end` enables iterable JSON conversion.

This avoids hard compile failures when a type lacks one conversion path.

## 4.2 In `include/stringify.h`

Traits decide how values are converted to string representations:

1. `to_construct` path when available.
2. `to_string`/`std::to_string` path when available.
3. Fallback path otherwise.

Again, compile-time selection is done with `enable_if` using traits from `sfinae.h`.

## 5. `is_empty`

```cpp
template<typename... T>
struct is_empty : std::false_type{};

template<>
struct is_empty<> : std::true_type{};
```

A tiny trait that is `true` only for an empty parameter pack.

## 6. Tuple Utility Functions

These are generic utilities unrelated to detection, but colocated in this file.

### `index_tuple`

```cpp
template<size_t... Args>
auto index_tuple(std::index_sequence<Args...>) {
    return std::make_tuple(Args...);
}

template<size_t N>
auto index_tuple() {
    return index_tuple(std::make_index_sequence<N>());
}
```

Produces tuple `(0, 1, ..., N-1)`.

### `tuple_from`

```cpp
template<int index = 0, typename... Tuples>
auto tuple_from(const Tuples&... tuples) {
    return std::make_tuple(std::get<index>(tuples)...);
}
```

Extracts same-position elements from multiple tuples.

Example:

1. `tuple_from<1>((a,b), (c,d), (e,f)) -> (b,d,f)`

### `zip`

```cpp
template<typename T, T... Indices, typename... Tuples>
auto zip(std::integer_sequence<T, Indices...>, const Tuples&... tuples) {
    return std::make_tuple(tuple_from<Indices>(tuples...)...);
}

template<typename... Args, typename... Tuples>
auto zip(const std::tuple<Args...>& tuple, const Tuples&... tuples) {
    return zip(std::index_sequence_for<Args...>(), tuple, tuples...);
}
```

Groups tuples by index.

Example:

1. `zip((a,b,c), (x,y,z)) -> ((a,x), (b,y), (c,z))`

### `for_each`

```cpp
template<typename Func, typename... Args>
inline void for_each(const std::tuple<Args...>& tuple, Func&& func) {
    std::apply([&func](auto... tuples){
        (..., std::apply(func, tuples));
    }, zip(index_tuple<sizeof...(Args)>(), tuple));
}

template<typename Func, typename Arg1, typename Args2>
inline void for_each(const std::pair<Arg1, Args2>& pair, Func&& func) {
    func(0, pair.first);
    func(1, pair.second);
}
```

Applies callable `func(index, value)` to each tuple element (or both pair elements).

## 7. Why this file exists in C++17 style

In C++20, many of these checks can be expressed more directly using `requires` and concepts.
This project is written in a C++17-compatible style, so SFINAE probes + traits are used instead.

## 8. Summary

1. `sfinae.h` provides compile-time type capability detection and tuple utilities.
2. `sfinae_bool` is a helper base to make logical composition (`operator*`) concise.
3. Traits from this file drive overload selection in `json.h` and `stringify.h`.
